// ----------------------------------------------------------------------------------------------
// Copyright (c) Mattias Högström.
// ----------------------------------------------------------------------------------------------
// This source code is subject to terms and conditions of the Microsoft Public License. A 
// copy of the license can be found in the License.html file at the root of this distribution. 
// If you cannot locate the Microsoft Public License, please send an email to 
// dlr@microsoft.com. By using this source code in any fashion, you are agreeing to be bound 
// by the terms of the Microsoft Public License.
// ----------------------------------------------------------------------------------------------
// You must not remove this notice, or any other, from this software.
// ----------------------------------------------------------------------------------------------

#include <Windows.h>
#include <objbase.h>
#include <tlhelp32.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "DiagProfiler.h"
#include "FileLogger.h"
#include "CallstackSampler.h"
#include "NativeStackwalker.h"
#include "ProfilerCommandServer.h"
#include "PerformanceCounter.h"
#include "DiagProfilerServer.h"
#include "DiagGlobals.h"

#define ARRAY_SIZE(s) (sizeof(s) / sizeof(s[0]))
#define NAME_BUFFER_SIZE 1024

using namespace std;


DiagProfiler* g_diagProfiler = NULL;

static DiagProfilerServer g_profilerServer;


void CALLBACK DiagProfiler::OnSample(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	volatile static long lockVariable = 0;
	if (g_diagProfiler != nullptr)
	{
        // If the OnSample requires more time than the Sample interval
        // OnSample will be called from another thread.
		if (InterlockedCompareExchangeAcquire(&lockVariable, 1, 0) == 0)
		{
			PerformanceCounter perfCounter;
			perfCounter.Start();

			g_diagProfiler->SampleCallstack();
			
            perfCounter.Stop();

            // TODO: This profiler is far from optimized
            // This performance counter can be used to measure execution time down to ticks level.
            g_debugLogger << "PERF - ListProcessThreads Ticks = " << dec << perfCounter.GetDurationInTicks() << endl;
			if (InterlockedCompareExchangeRelease(&lockVariable, 0, 1) == 0)
				throw "Lock Inconsistency";
		}
		else
		{
            // If OnSample is called while we are already executing a OnSample
            // Just Log and return;
            g_debugLogger << "PERF - Overlapped call to OnSample" << endl;
		}
	}
	else
	{
		throw "OnSample : g_diagProfiler == null";
	}	
}



STDMETHODIMP DiagProfiler::SampleCallstack()
{
	m_currentSampleCount++;
	if(m_maxSamples == m_currentSampleCount)
	{
		StopSampling();  // running = false, timer delete
		g_stackLogger.Close();
        g_debugLogger << "Done - MaxSamples = " << m_currentSampleCount << endl;
		return S_OK;
	}

	// TODO Optimize this
	// Cache FunctionIds and their strings

	PerformanceCounter perfCounter;

	DWORD notMe = GetCurrentThreadId();

	// Copying list
	// to avoid strange crash in iterator when stopping sampling

	std::list<DWORD> localList = std::list<DWORD>(m_nativeThreads);


	std::list<DWORD>::iterator iter_begin = localList.begin();
	std::list<DWORD>::iterator iter_end = localList.end();
	int listSize = localList.size();
	for(std::list<DWORD>::iterator list_iter = iter_begin; list_iter != iter_end; list_iter++)
	{
		ThreadID threadId = *list_iter; //win32 threadId
		ThreadID managedThreadId = m_native2ManagedMap[threadId]; //managed threadId

        if (managedThreadId == 0)
        {
            if (m_onlyManagedThreads)
            {
                continue;
            }
        }
        

		if (threadId == notMe)
		{
			g_debugLogger << "Skipping myself 0x" << hex << threadId << endl;
			continue;
		}
		if (threadId == g_profilerServer.GetThreadId())
		{
            g_debugLogger << "Skipping pipe com thread 0x" << hex << threadId << endl;
		}

		if (threadId == 0)
			__debugbreak();
		
		g_debugLogger.WriteLine("----- BEGIN CALLSTACK -----");
        g_debugLogger << "ThreadId = 0x" << threadId << endl;
		perfCounter.Start();
		
		g_debugLoggerVerbose.WriteLine("SampleCallstack GetStackTrace");
		HRESULT hr = m_callstackSampler.GetStackTrace(m_corProfilerInfo2, managedThreadId, threadId);
		perfCounter.Stop();
        g_debugLogger << "PERF - GetStackTrace Ticks = " << dec << perfCounter.GetDurationInTicks() << endl;
        g_debugLoggerVerbose.WriteLine("-----  END CALLSTACK  -----");
	}
	return S_OK;
}

HRESULT DiagProfiler::SetEventMask()
{
	//COR_PRF_MONITOR_NONE	= 0,
	//COR_PRF_MONITOR_FUNCTION_UNLOADS	= 0x1,
	//COR_PRF_MONITOR_CLASS_LOADS	= 0x2,
	//COR_PRF_MONITOR_MODULE_LOADS	= 0x4,
	//COR_PRF_MONITOR_ASSEMBLY_LOADS	= 0x8,
	//COR_PRF_MONITOR_APPDOMAIN_LOADS	= 0x10,
	//COR_PRF_MONITOR_JIT_COMPILATION	= 0x20,
	//COR_PRF_MONITOR_EXCEPTIONS	= 0x40,
	//COR_PRF_MONITOR_GC	= 0x80,
	//COR_PRF_MONITOR_OBJECT_ALLOCATED	= 0x100,
	//COR_PRF_MONITOR_THREADS	= 0x200,
	//COR_PRF_MONITOR_REMOTING	= 0x400,
	//COR_PRF_MONITOR_CODE_TRANSITIONS	= 0x800,
	//COR_PRF_MONITOR_ENTERLEAVE	= 0x1000,
	//COR_PRF_MONITOR_CCW	= 0x2000,
	//COR_PRF_MONITOR_REMOTING_COOKIE	= 0x4000 | COR_PRF_MONITOR_REMOTING,
	//COR_PRF_MONITOR_REMOTING_ASYNC	= 0x8000 | COR_PRF_MONITOR_REMOTING,
	//COR_PRF_MONITOR_SUSPENDS	= 0x10000,
	//COR_PRF_MONITOR_CACHE_SEARCHES	= 0x20000,
	//COR_PRF_MONITOR_CLR_EXCEPTIONS	= 0x1000000,
	//COR_PRF_MONITOR_ALL	= 0x107ffff,
	//COR_PRF_ENABLE_REJIT	= 0x40000,
	//COR_PRF_ENABLE_INPROC_DEBUGGING	= 0x80000,
	//COR_PRF_ENABLE_JIT_MAPS	= 0x100000,
	//COR_PRF_DISABLE_INLINING	= 0x200000,
	//COR_PRF_DISABLE_OPTIMIZATIONS	= 0x400000,
	//COR_PRF_ENABLE_OBJECT_ALLOCATED	= 0x800000,
	
	// New in VS2005
	//COR_PRF_ENABLE_FUNCTION_ARGS	= 0x2000000,
	//COR_PRF_ENABLE_FUNCTION_RETVAL	= 0x4000000,
	//COR_PRF_ENABLE_FRAME_INFO	= 0x8000000,
	//COR_PRF_ENABLE_STACK_SNAPSHOT	= 0x10000000,
	//COR_PRF_USE_PROFILE_IMAGES	= 0x20000000,
	// End New in VS2005
	
	//COR_PRF_ALL	= 0x3fffffff,
	//COR_PRF_MONITOR_IMMUTABLE	= COR_PRF_MONITOR_CODE_TRANSITIONS | COR_PRF_MONITOR_REMOTING | COR_PRF_MONITOR_REMOTING_COOKIE | COR_PRF_MONITOR_REMOTING_ASYNC | COR_PRF_MONITOR_GC | COR_PRF_ENABLE_REJIT | COR_PRF_ENABLE_INPROC_DEBUGGING | COR_PRF_ENABLE_JIT_MAPS | COR_PRF_DISABLE_OPTIMIZATIONS | COR_PRF_DISABLE_INLINING | COR_PRF_ENABLE_OBJECT_ALLOCATED | COR_PRF_ENABLE_FUNCTION_ARGS | COR_PRF_ENABLE_FUNCTION_RETVAL | COR_PRF_ENABLE_FRAME_INFO | COR_PRF_ENABLE_STACK_SNAPSHOT | COR_PRF_USE_PROFILE_IMAGES

	// set the event mask 
	//DWORD eventMask = (DWORD)(COR_PRF_MONITOR_ENTERLEAVE);
	DWORD eventMask = COR_PRF_MONITOR_NONE;
	eventMask |= COR_PRF_MONITOR_THREADS; // create / delete thread
    
    //
    //eventMask |= COR_PRF_MONITOR_CODE_TRANSITIONS;
    //eventMask |= COR_PRF_MONITOR_SUSPENDS;

	eventMask |= COR_PRF_ENABLE_STACK_SNAPSHOT;
//  eventMask |= COR_PRF_MONITOR_SUSPENDS;
//	eventMask |= COR_PRF_DISABLE_INLINING;
//	eventMask |= COR_PRF_DISABLE_OPTIMIZATIONS;
//	eventMask | = COR_PRF_MONITOR_IMMUTABLE;

	return m_corProfilerInfo->SetEventMask(eventMask);
}


DiagProfiler::DiagProfiler()
{
	m_profilerTimer = INVALID_HANDLE_VALUE;
	m_runProfiling = true;
	m_samplingRate = SAMPLING_DEFAULT_RATE;
	m_maxSamples = SAMPLING_ULIMITED_SAMPLES;
	m_currentSampleCount = 0;
	m_onlyManagedThreads = false;

	m_nRefCount = 0;
	g_diagProfiler = nullptr;
	m_corProfilerInfo = nullptr;
	m_corProfilerInfo2 = nullptr;
}

DiagProfiler::~DiagProfiler()
{
	m_corProfilerInfo = nullptr;
	m_corProfilerInfo2 = nullptr;
	g_diagProfiler = nullptr;
	StopProfilerTimer();
}

STDMETHODIMP DiagProfiler::QueryInterface(
                                    REFIID riid , 
                                    void **ppObj)
{
	LPOLESTR clsid = nullptr;

	HRESULT hr = StringFromCLSID(riid, &clsid);
    if (SUCCEEDED(hr))
    {
        std::wstring clsidString(clsid);
        g_debugLogger << "DiagProfiler::QueryInterface(" << ConvertStlString(clsidString).c_str() << ")" << endl;
        ::CoTaskMemFree(clsid);
    }
    if (riid == IID_IUnknown)
    {
		*ppObj = this; 
		AddRef() ;
		g_debugLogger.WriteLine("QueryInterface -> IUnknown");
		return S_OK;
    }

    if (riid == IID_IDiagProfiler)
	{
		*ppObj = static_cast<IDiagProfiler*>(this) ;
		AddRef() ;
		g_debugLogger.WriteLine("QueryInterface -> IDiagProfiler");
		return S_OK;
	}

	if (riid == IID_ICorProfilerCallback)
	{
		*ppObj = static_cast<ICorProfilerCallback*>(this) ;
		AddRef() ;
		g_debugLogger.WriteLine("QueryInterface -> ICorProfilerCallback");
		return S_OK;
	}

	if (riid == IID_ICorProfilerCallback2)
	{
		*ppObj = static_cast<ICorProfilerCallback2*>(this) ;
		AddRef();
		g_debugLogger.WriteLine("QueryInterface -> ICorProfilerCallback2");
		return S_OK;
	}

	if (riid == IID_ICorProfilerCallback3)
	{
		*ppObj = dynamic_cast<ICorProfilerCallback3*>(this) ;
		AddRef();
		g_debugLogger.WriteLine("QueryInterface -> ICorProfilerCallback");
		return S_OK;
	}

	if (riid == IID_ICorProfilerInfo)
	{
		g_debugLogger.WriteLine("QueryInterface -> ICorProfilerInfo");
		*ppObj = m_corProfilerInfo;
		return S_OK;
	}

	if (riid == IID_ICorProfilerInfo2)
	{
		g_debugLogger.WriteLine("QueryInterface -> ICorProfilerInfo2");
		*ppObj = m_corProfilerInfo2;
		return S_OK;
	}

    *ppObj = NULL ;
    g_debugLogger.WriteLine("QueryInterface -> E_NOINTERFACE");
    return E_NOINTERFACE ;
}

ULONG STDMETHODCALLTYPE DiagProfiler::AddRef()
{
	return InterlockedIncrement(&m_nRefCount) ;
}

ULONG STDMETHODCALLTYPE DiagProfiler::Release()
{     
	long nRefCount=0;
	nRefCount=InterlockedDecrement(&m_nRefCount) ;
	if (nRefCount == 0) 
	{
		g_debugLogger.Close();
	}
	return nRefCount;
}

HRESULT DiagProfiler::GetSamplingRate(int* milliSeconds)
{
	*milliSeconds = m_samplingRate;
	return S_OK;
}

HRESULT DiagProfiler::GetNoOfSamples(int* noSamples)
{
	*noSamples = m_currentSampleCount;
	return S_OK;
}

HRESULT DiagProfiler::SetSamplingRate(int milliSeconds)
{
	if ((20 <= milliSeconds) && (milliSeconds <= 1000))
	{
		m_samplingRate = milliSeconds;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT DiagProfiler::SetNoOfSamples(int noSamples)
{
	if (noSamples > 0)
	{
		m_maxSamples = noSamples;
		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}

BOOL DiagProfiler::StartProfilerTimer()
{
	g_debugLogger.WriteLine("Starting OnSample Timer");
	m_currentSampleCount = 0;

	if (m_profilerTimer != INVALID_HANDLE_VALUE)
		return S_FALSE;
	g_debugLogger.WriteLine("Starting OnSample Timer : Creating Timer");
	return CreateTimerQueueTimer(&m_profilerTimer, NULL /* Use Default Queue*/, OnSample, this, 100, m_samplingRate /*0*/ /*500*/, WT_EXECUTEONLYONCE);
}

BOOL DiagProfiler::StopProfilerTimer()
{
	g_debugLogger.WriteLine("Stopping OnSample Timer");
	if (m_profilerTimer == INVALID_HANDLE_VALUE)
		return FALSE;
	g_debugLogger.WriteLine("Stopping OnSample Timer : Deleting Timer");
	BOOL status = DeleteTimerQueueTimer(NULL, m_profilerTimer, NULL);
	if (status)
	{
		m_profilerTimer = INVALID_HANDLE_VALUE;
	}
	else
	{
        DWORD last = GetLastError();
        switch (last)
        {
        case ERROR_IO_PENDING:
            // OK. Will be deleted when callback has finised
      		g_debugLogger.WriteLine("Stopping OnSample Timer : OK (ERROR_IO_PENDING)");
            status = TRUE;
            break;
        default:
            g_debugLogger.WriteLine("Stopping OnSample Timer : Failed");
            g_debugLogger << "Last Error = 0x" << hex << last << endl;
        }
	}
	return status;
}

BOOL DiagProfiler::UpdateThreadList(void)
{
	HRESULT hr = NativeStackwalker::ListProcessThreads(GetCurrentProcessId(), m_nativeThreads);
	return hr == S_OK;
}

HRESULT DiagProfiler::StartSampling(void)
{
	g_debugLogger.WriteLine("StartSampling");
	UpdateThreadList();

	if (m_runProfiling)
		return S_FALSE;
	if (m_samplingRate > 0)
	{
		m_runProfiling = true;
		if(StartProfilerTimer())
			return S_OK;
		m_runProfiling = false;
		return S_FALSE;
	}
	else
	{
		return S_FALSE;
	}
}

HRESULT DiagProfiler::StopSampling(void)
{
	if (m_runProfiling)
	{
		m_runProfiling = false;
		return StopProfilerTimer() ? S_OK : S_FALSE;
	}
	else
	{
		return S_FALSE;
	}
}

STDMETHODIMP DiagProfiler::ProfilerAttachComplete(void)
{
	g_debugLogger.WriteLine("ProfilerAttachComplete");
	return S_OK;
}
        
STDMETHODIMP DiagProfiler::ProfilerDetachSucceeded(void)
{
	g_debugLogger.WriteLine("ProfilerDetachSucceeded");
	return S_OK;
};

HANDLE g_connectTimer = INVALID_HANDLE_VALUE;

//
//  When we attach with ICLRProfiling::AttachProfiler
//  We listen for a client to connect through a Pipe
//  This function cancels the timer, and configures a command server
//
void CALLBACK DiagProfiler::OnClientConnect(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	g_debugLogger.WriteLine("DiagProfiler::OnAttach");
	BOOL deleted = DeleteTimerQueueTimer(NULL, g_connectTimer, NULL);
	// BOOL closed = CloseHandle(attachTimer);
	// Don't Close. It crashes

	g_connectTimer = INVALID_HANDLE_VALUE;

    //g_profilerServer.SetDebugLogger(&g_debugLogger);

    BOOL connected = g_profilerServer.Start(g_diagProfiler, 15000); //g_commandServer.Start(15000);
	if (!connected)
	{
		g_debugLogger.WriteLine("DiagProfiler::OnAttach - Not Connected");
	}
	else
	{
		g_debugLogger.WriteLine("DiagProfiler::OnAttach - Connected");
	}
}


BOOL DiagProfiler::RunWithGUI()
{
	m_runProfiling = false;
	m_maxSamples = SAMPLING_ULIMITED_SAMPLES;
	g_debugLogger.WriteLine("RunWithGUI - Timer Created");
	BOOL result = CreateTimerQueueTimer(&g_connectTimer, NULL, OnClientConnect, NULL, 500, 0, WT_EXECUTEONLYONCE);
	return result;
}

//
// Called When a profiler is started using ICLRProfiling::AttachProfiler
//
STDMETHODIMP DiagProfiler::InitializeForAttach( 
            /* [in] */ IUnknown *pCorProfilerInfoUnk,
            /* [in] */ void *pvClientData,
            /* [in] */ UINT cbClientData)
{
	g_debugLogger.WriteLine("InitializeForAttach");

	m_runProfiling = false;


	HRESULT hr = this->Initialize(pCorProfilerInfoUnk);
    // reenable the debugoutput
    // previsously turned off by Initialize()
    g_diagInit.m_debugLogger.Echo2OutputDebug(true);

   
	if (SUCCEEDED(hr))
	{
		if(RunWithGUI())
		{
			;
		}
	}
	return hr;
}

//
// Called when a profiler is attached using environment variables
//
STDMETHODIMP DiagProfiler::Initialize(IUnknown *pICorProfilerInfoUnk)
{
	std::cout << "come in " << endl;
	g_debugLogger.WriteLine("DiagProfiler::Initialize()");
	g_diagProfiler = this;

	// get the ICorProfilerInfo interface
    HRESULT hr = pICorProfilerInfoUnk->QueryInterface(IID_ICorProfilerInfo, (LPVOID*)&m_corProfilerInfo);
    if (FAILED(hr))
    {
        g_debugLogger.WriteLine("Error: Failed to get ICorProfilerInfo");
        return E_FAIL;
    }
    else
    {
        g_debugLogger.WriteLine("Got ICorProfilerInfo");
    }

    hr = pICorProfilerInfoUnk->QueryInterface(IID_ICorProfilerInfo2, (LPVOID*)&m_corProfilerInfo2);
    if (FAILED(hr))
	{
		m_corProfilerInfo2 = nullptr;
		g_debugLogger.WriteLine("Error: Failed to get ICorProfiler2");
	}
	else
	{
		g_debugLogger.WriteLine("Got ICorProfilerInfo2");
	}

	// Tell the profiler API which events we want to listen to
	// Some events fail when we attach afterwards

	hr = SetEventMask();
    if (FAILED(hr))
    {
        g_debugLogger.WriteLine("Error: Failed to set event mask");
    }
    else
    {
        g_debugLogger.WriteLine("SetEventMask()");
    }

	g_debugLogger.WriteLine("Successfully initialized profiling");
	
	// Add running threads
	// Needed if we attach to the process

	UpdateThreadList();
    
    std::string envSymbolPath = std::string("DIAG_PRF_SYMBOLPATH");
    std::string envWithGUI = std::string("DIAG_PRF_RUN_WITH_GUI");
	std::string envOnlyManagedThreads = std::string("DIAG_PRF_ONLY_MANAGED_THREADS");
	std::string withGUI = GetEnvVar(envWithGUI);
	std::string onlyManaged = GetEnvVar(envOnlyManagedThreads);
    std::string strOne = std::string("1");
    std::string strZero = std::string("0");

    if (onlyManaged.compare(strOne) == 0)
	{
		m_onlyManagedThreads = true;
        g_debugLogger << "Will profile only managed threads" << endl;
	}
    else if (onlyManaged.compare(strZero) == 0)
    {
        g_debugLogger << "Will Profile all threads" << endl;
    }
    else
    {
        g_debugLogger << envOnlyManagedThreads.c_str() << " = <Undefined>" << endl;
        g_debugLogger << "Will profile all threads" << endl;
    }

    if (withGUI.compare(strOne) == 0)
	{
        g_debugLogger << "Running with GUI" << endl;
		RunWithGUI();
	}
    else
    {
        g_debugLogger << "Running without GUI" << endl;
    }
	if (m_runProfiling)
	{
		if (FAILED(StartProfilerTimer()))
			return E_FAIL; 
	}
    // We do not need echo to OutputDebugString
    g_diagInit.m_debugLogger.Echo2OutputDebug(false);

    return S_OK;
}

STDMETHODIMP DiagProfiler::ThreadAssignedToOSThread(ThreadID managedThreadID, DWORD osThreadID) 
{
    g_debugLogger << "ThreadAssignedToOSThread m=" << managedThreadID << " os=" << osThreadID << endl;
	m_managed2NativeMap[managedThreadID] = osThreadID;
	m_native2ManagedMap[osThreadID] = managedThreadID;
    return S_OK;
}

//
//
//   Empty method bodies
//
//

STDMETHODIMP DiagProfiler::Shutdown()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::AppDomainCreationStarted(AppDomainID appDomainID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::AppDomainCreationFinished(AppDomainID appDomainID, HRESULT hrStatus)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::AppDomainShutdownStarted(AppDomainID appDomainID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::AppDomainShutdownFinished(AppDomainID appDomainID, HRESULT hrStatus)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::AssemblyLoadStarted(AssemblyID assemblyID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::AssemblyLoadFinished(AssemblyID assemblyID, HRESULT hrStatus)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::AssemblyUnloadStarted(AssemblyID assemblyID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::AssemblyUnloadFinished(AssemblyID assemblyID, HRESULT hrStatus)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ModuleLoadStarted(ModuleID moduleID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ModuleLoadFinished(ModuleID moduleID, HRESULT hrStatus)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ModuleUnloadStarted(ModuleID moduleID)
{
    return S_OK;
}
	  
STDMETHODIMP DiagProfiler::ModuleUnloadFinished(ModuleID moduleID, HRESULT hrStatus)
{
	return S_OK;
}

STDMETHODIMP DiagProfiler::ModuleAttachedToAssembly(ModuleID moduleID, AssemblyID assemblyID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ClassLoadStarted(ClassID classID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ClassLoadFinished(ClassID classID, HRESULT hrStatus)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ClassUnloadStarted(ClassID classID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ClassUnloadFinished(ClassID classID, HRESULT hrStatus)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::FunctionUnloadStarted(FunctionID functionID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::JITCompilationStarted(FunctionID functionID, BOOL fIsSafeToBlock)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::JITCompilationFinished(FunctionID functionID, HRESULT hrStatus, BOOL fIsSafeToBlock)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::JITCachedFunctionSearchStarted(FunctionID functionID, BOOL *pbUseCachedFunction)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::JITCachedFunctionSearchFinished(FunctionID functionID, COR_PRF_JIT_CACHE result)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::JITFunctionPitched(FunctionID functionID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::JITInlining(FunctionID callerID, FunctionID calleeID, BOOL *pfShouldInline)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::UnmanagedToManagedTransition(FunctionID functionID, COR_PRF_TRANSITION_REASON reason)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ManagedToUnmanagedTransition(FunctionID functionID, COR_PRF_TRANSITION_REASON reason)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ThreadCreated(ThreadID threadID)
{
	m_managedThreads.push_front(threadID);
    return S_OK;
}

STDMETHODIMP DiagProfiler::ThreadDestroyed(ThreadID threadID)
{
	m_managedThreads.remove(threadID);
    return S_OK;
}

STDMETHODIMP DiagProfiler::RemotingClientInvocationStarted()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::RemotingClientSendingMessage(GUID *pCookie, BOOL fIsAsync)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::RemotingClientReceivingReply(GUID *pCookie, BOOL fIsAsync)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::RemotingClientInvocationFinished()
{
	return S_OK;
}

STDMETHODIMP DiagProfiler::RemotingServerReceivingMessage(GUID *pCookie, BOOL fIsAsync)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::RemotingServerInvocationStarted()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::RemotingServerInvocationReturned()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::RemotingServerSendingReply(GUID *pCookie, BOOL fIsAsync)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::RuntimeSuspendFinished()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::RuntimeSuspendAborted()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::RuntimeResumeStarted()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::RuntimeResumeFinished()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::RuntimeThreadSuspended(ThreadID threadID)
{
	return S_OK;
}

STDMETHODIMP DiagProfiler::RuntimeThreadResumed(ThreadID threadID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::MovedReferences(ULONG cmovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], ULONG cObjectIDRangeLength[])
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ObjectAllocated(ObjectID objectID, ClassID classID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ObjectsAllocatedByClass(ULONG classCount, ClassID classIDs[], ULONG objects[])
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ObjectReferences(ObjectID objectID, ClassID classID, ULONG objectRefs, ObjectID objectRefIDs[])
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::RootReferences(ULONG rootRefs, ObjectID rootRefIDs[])
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionThrown(ObjectID thrownObjectID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionUnwindFunctionEnter(FunctionID functionID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionUnwindFunctionLeave()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionSearchFunctionEnter(FunctionID functionID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionSearchFunctionLeave()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionSearchFilterEnter(FunctionID functionID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionSearchFilterLeave()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionSearchCatcherFound(FunctionID functionID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionCLRCatcherFound()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionCLRCatcherExecute()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionOSHandlerEnter(FunctionID functionID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionOSHandlerLeave(FunctionID functionID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionUnwindFinallyEnter(FunctionID functionID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionUnwindFinallyLeave()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionCatcherEnter(FunctionID functionID,
    											 ObjectID objectID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ExceptionCatcherLeave()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::COMClassicVTableCreated(ClassID wrappedClassID, REFGUID implementedIID, void *pVTable, ULONG cSlots)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::COMClassicVTableDestroyed(ClassID wrappedClassID, REFGUID implementedIID, void *pVTable)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::ThreadNameChanged(ThreadID threadID, ULONG cchName, WCHAR name[])
{
	return S_OK;
}

STDMETHODIMP DiagProfiler::GarbageCollectionStarted(int cGenerations, BOOL generationCollected[], COR_PRF_GC_REASON reason)
{
	return S_OK;
}

STDMETHODIMP DiagProfiler::SurvivingReferences(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], ULONG cObjectIDRangeLength[])
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::GarbageCollectionFinished()
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::FinalizeableObjectQueued(DWORD finalizerFlags, ObjectID objectID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::RootReferences2(ULONG cRootRefs, ObjectID rootRefIDs[], COR_PRF_GC_ROOT_KIND rootKinds[], COR_PRF_GC_ROOT_FLAGS rootFlags[], UINT_PTR rootIDs[])
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::HandleCreated(GCHandleID handleID, ObjectID initialObjectID)
{
    return S_OK;
}

STDMETHODIMP DiagProfiler::HandleDestroyed(GCHandleID handleID)
{
    return S_OK;
}
