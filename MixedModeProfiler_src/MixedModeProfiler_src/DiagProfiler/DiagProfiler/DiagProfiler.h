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

#pragma once

#include <Windows.h>
#include <cor.h>
#include <corprof.h>
#include <map>
#include <list>
#include "IDiagProfiler_h.h"
#include "CallstackSampler.h"
#include "IDiagProfilerClient.h"

extern long g_comObjectsInUse;

class DiagProfiler :
//	public IUnknown,
//	public ICorProfilerCallback,
//	public ICorProfilerCallback2,
	public ICorProfilerCallback3,
	public IDiagProfiler,
    public IDiagProfilerClient
{
public:
	DiagProfiler();
	~DiagProfiler();
    STDMETHOD(QueryInterface)(REFIID riid, void **ppObj);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    STDMETHOD(Initialize)(IUnknown *pICorProfilerInfoUnk);
    STDMETHOD(Shutdown)();
   	STDMETHOD(AppDomainCreationStarted)(AppDomainID appDomainID);
	STDMETHOD(AppDomainCreationFinished)(AppDomainID appDomainID, HRESULT hrStatus);
    STDMETHOD(AppDomainShutdownStarted)(AppDomainID appDomainID);
	STDMETHOD(AppDomainShutdownFinished)(AppDomainID appDomainID, HRESULT hrStatus);
   	STDMETHOD(AssemblyLoadStarted)(AssemblyID assemblyID);
	STDMETHOD(AssemblyLoadFinished)(AssemblyID assemblyID, HRESULT hrStatus);
    STDMETHOD(AssemblyUnloadStarted)(AssemblyID assemblyID);
	STDMETHOD(AssemblyUnloadFinished)(AssemblyID assemblyID, HRESULT hrStatus);
   	STDMETHOD(ModuleLoadStarted)(ModuleID moduleID);
	STDMETHOD(ModuleLoadFinished)(ModuleID moduleID, HRESULT hrStatus);
    STDMETHOD(ModuleUnloadStarted)(ModuleID moduleID);
	STDMETHOD(ModuleUnloadFinished)(ModuleID moduleID, HRESULT hrStatus);
	STDMETHOD(ModuleAttachedToAssembly)(ModuleID moduleID, AssemblyID assemblyID);
    STDMETHOD(ClassLoadStarted)(ClassID classID);
    STDMETHOD(ClassLoadFinished)(ClassID classID, HRESULT hrStatus);
 	STDMETHOD(ClassUnloadStarted)(ClassID classID);
	STDMETHOD(ClassUnloadFinished)(ClassID classID, HRESULT hrStatus);
	STDMETHOD(FunctionUnloadStarted)(FunctionID functionID);
    STDMETHOD(JITCompilationStarted)(FunctionID functionID, BOOL fIsSafeToBlock);
    STDMETHOD(JITCompilationFinished)(FunctionID functionID, HRESULT hrStatus, BOOL fIsSafeToBlock);
    STDMETHOD(JITCachedFunctionSearchStarted)(FunctionID functionID, BOOL *pbUseCachedFunction);
	STDMETHOD(JITCachedFunctionSearchFinished)(FunctionID functionID, COR_PRF_JIT_CACHE result);
    STDMETHOD(JITFunctionPitched)(FunctionID functionID);
    STDMETHOD(JITInlining)(FunctionID callerID, FunctionID calleeID, BOOL *pfShouldInline);
    STDMETHOD(ThreadCreated)(ThreadID threadID);
    STDMETHOD(ThreadDestroyed)(ThreadID threadID);
    STDMETHOD(ThreadAssignedToOSThread)(ThreadID managedThreadID, DWORD osThreadID);
    STDMETHOD(RemotingClientInvocationStarted)();
    STDMETHOD(RemotingClientSendingMessage)(GUID *pCookie, BOOL fIsAsync);
    STDMETHOD(RemotingClientReceivingReply)(GUID *pCookie, BOOL fIsAsync);
    STDMETHOD(RemotingClientInvocationFinished)();
    STDMETHOD(RemotingServerReceivingMessage)(GUID *pCookie, BOOL fIsAsync);
    STDMETHOD(RemotingServerInvocationStarted)();
    STDMETHOD(RemotingServerInvocationReturned)();
    STDMETHOD(RemotingServerSendingReply)(GUID *pCookie, BOOL fIsAsync);
	STDMETHOD(UnmanagedToManagedTransition)(FunctionID functionID, COR_PRF_TRANSITION_REASON reason);
    STDMETHOD(ManagedToUnmanagedTransition)(FunctionID functionID, COR_PRF_TRANSITION_REASON reason);
    STDMETHOD(RuntimeSuspendStarted)(COR_PRF_SUSPEND_REASON suspendReason);
    STDMETHOD(RuntimeSuspendFinished)();
    STDMETHOD(RuntimeSuspendAborted)();
    STDMETHOD(RuntimeResumeStarted)();
    STDMETHOD(RuntimeResumeFinished)();
    STDMETHOD(RuntimeThreadSuspended)(ThreadID threadid);
    STDMETHOD(RuntimeThreadResumed)(ThreadID threadid);
    STDMETHOD(MovedReferences)(ULONG cmovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], ULONG cObjectIDRangeLength[]);
    STDMETHOD(ObjectAllocated)(ObjectID objectID, ClassID classID);
    STDMETHOD(ObjectsAllocatedByClass)(ULONG classCount, ClassID classIDs[], ULONG objects[]);
    STDMETHOD(ObjectReferences)(ObjectID objectID, ClassID classID, ULONG cObjectRefs, ObjectID objectRefIDs[]);
    STDMETHOD(RootReferences)(ULONG cRootRefs, ObjectID rootRefIDs[]);
    STDMETHOD(ExceptionThrown)(ObjectID thrownObjectID);
    STDMETHOD(ExceptionSearchFunctionEnter)(FunctionID functionID);
    STDMETHOD(ExceptionSearchFunctionLeave)();
    STDMETHOD(ExceptionSearchFilterEnter)(FunctionID functionID);
    STDMETHOD(ExceptionSearchFilterLeave)();
    STDMETHOD(ExceptionSearchCatcherFound)(FunctionID functionID);
    STDMETHOD(ExceptionCLRCatcherFound)();
    STDMETHOD(ExceptionCLRCatcherExecute)();
    STDMETHOD(ExceptionOSHandlerEnter)(FunctionID functionID);
    STDMETHOD(ExceptionOSHandlerLeave)(FunctionID functionID);
    STDMETHOD(ExceptionUnwindFunctionEnter)(FunctionID functionID);
    STDMETHOD(ExceptionUnwindFunctionLeave)();
    STDMETHOD(ExceptionUnwindFinallyEnter)(FunctionID functionID);
    STDMETHOD(ExceptionUnwindFinallyLeave)();
    STDMETHOD(ExceptionCatcherEnter)(FunctionID functionID, ObjectID objectID);
    STDMETHOD(ExceptionCatcherLeave)();
    STDMETHOD(COMClassicVTableCreated)(ClassID wrappedClassID, REFGUID implementedIID, void *pVTable, ULONG cSlots);
    STDMETHOD(COMClassicVTableDestroyed)(ClassID wrappedClassID, REFGUID implementedIID, void *pVTable);
	STDMETHOD(ThreadNameChanged)(ThreadID threadId, ULONG cchName, WCHAR name[]);
	STDMETHOD(GarbageCollectionStarted)(int cGenerations, BOOL generationCollected[], COR_PRF_GC_REASON reason);
    STDMETHOD(SurvivingReferences)(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], ULONG cObjectIDRangeLength[]);
    STDMETHOD(GarbageCollectionFinished)();
    STDMETHOD(FinalizeableObjectQueued)(DWORD finalizerFlags, ObjectID objectID);
    STDMETHOD(RootReferences2)(ULONG cRootRefs, ObjectID rootRefIds[], COR_PRF_GC_ROOT_KIND rootKinds[], COR_PRF_GC_ROOT_FLAGS rootFlags[], UINT_PTR rootIds[]);
    STDMETHOD(HandleCreated)(GCHandleID handleId, ObjectID initialObjectId);
    STDMETHOD(HandleDestroyed)(GCHandleID handleId);
	STDMETHOD(InitializeForAttach)(IUnknown *pCorProfilerInfoUnk, void *pvClientData, UINT cbClientData);
    STDMETHOD(ProfilerAttachComplete)(void);        
    STDMETHOD(ProfilerDetachSucceeded)(void);

    // My own API
    STDMETHOD(SetSamplingRate)(int samplingRate);
    STDMETHOD(GetSamplingRate)(int* samplingRate);
    STDMETHOD(SetNoOfSamples)(int noSamples);
    STDMETHOD(GetNoOfSamples)(int* noSamples);
    STDMETHOD(StartSampling)(void);
    STDMETHOD(StopSampling)(void);

	BOOL UpdateThreadList(void);
	BOOL StartProfilerTimer(void);
	BOOL StopProfilerTimer(void);
	BOOL RunWithGUI(void);

	HRESULT SetEventMask();	
	STDMETHOD(SampleCallstack)();

    private:
	static const int SAMPLING_ULIMITED_SAMPLES = 0;
	static const int SAMPLING_DEFAULT_SAMPLES = 6000;
	static const int SAMPLING_DEFAULT_RATE = 40;

	bool m_onlyManagedThreads;
	bool m_runProfiling;
	int m_samplingRate;
	int m_maxSamples;
	int m_currentSampleCount;
	HANDLE m_profilerTimer;
    long m_nRefCount;   //for managing the reference count

	ICorProfilerInfo* m_corProfilerInfo;
	ICorProfilerInfo2* m_corProfilerInfo2;

	std::list<ThreadID> m_managedThreads;
	std::list<DWORD> m_nativeThreads;
	std::map<ThreadID, DWORD> m_managed2NativeMap;
	std::map<DWORD, ThreadID> m_native2ManagedMap;
    CallstackSampler m_callstackSampler;
    static void CALLBACK OnClientConnect(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
    static void CALLBACK OnSample(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
};
