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
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <string>
#include <cor.h>
#include <corprof.h>
#include <corpub.h>
#include <cordebug.h>
#include <vector>
#include <assert.h>
#include <list>

#include "CallstackSampler.h"
#include "NativeStackwalker.h"
#include "DiagGlobals.h"

using namespace std;

struct StackWalkFrame
{
	bool IsManaged;
	bool IsPartial;
	FunctionID FuncId;
	DWORD64 EIP;
	DWORD64 EBP;
	DWORD64 ESP;
	std::string SymbolName;
};

const int NAME_BUFFER_SIZE = 512;
static ICorProfilerInfo2* g_info2 = NULL;
static std::list<StackWalkFrame> g_frames;
static std::list<DWORD64> g_nativeFrames;
static std::map<FunctionID, std::string> g_funcId2StringMap;
static int g_funcId2StringMapCacheHit = 0;

HRESULT __stdcall CallstackSampler::MyDoStackSnapshotCallback (
FunctionID funcId,
UINT_PTR ip,
COR_PRF_FRAME_INFO frameInfo,
ULONG32 contextSize,
BYTE context[],
void *clientData
)
{	
	HRESULT hr = S_OK;
	IMetaDataImport* metadata = 0;
	IMetaDataImport* metadata2 = 0;
	IMetaDataImport* metadata3 = 0;
    mdMethodDef methodToken = mdTypeDefNil;
	mdTypeDef classDef = mdTypeDefNil;
	mdTypeDef typeDefToken = mdTypeDefNil;
    WCHAR szFunction[NAME_BUFFER_SIZE];
	WCHAR szClass[NAME_BUFFER_SIZE];
	ULONG cchMethod;
	PCCOR_SIGNATURE sigBlob = NULL;
	ULONG sigBlobBytesCount = NULL;
	ULONG classLength = NAME_BUFFER_SIZE;
	if (g_info2 == nullptr)
		return S_FALSE;
	DWORD64 rip = ip;

	CONTEXT *contextData = reinterpret_cast<CONTEXT*>(context);

	if (funcId == 0)
	{		
		StackWalkFrame* newFrame = new StackWalkFrame();
		newFrame->IsManaged = false;
		newFrame->IsPartial = true;
		newFrame->FuncId = 0;
		newFrame->EIP = contextData->Eip;
		newFrame->EBP = contextData->Ebp;
		newFrame->ESP = contextData->Esp;
        //newFrame->SymbolName // Resolved later in ExpandFrames()
		g_frames.push_back(*newFrame);
		return S_OK;
	}
    std::string strEmpty;
    std::string funcIdString = g_funcId2StringMap[funcId];
    if (funcIdString.compare(strEmpty) != 0)
    {
        // FuncId already processed

		StackWalkFrame* newFrame = new StackWalkFrame();
		newFrame->IsManaged = true;
		newFrame->IsPartial = false;
		newFrame->FuncId = funcId;
		newFrame->EIP = contextData->Eip;
		newFrame->EBP = contextData->Ebp;
		newFrame->ESP = contextData->Esp;
        newFrame->SymbolName = funcIdString;
		g_frames.push_back(*newFrame);
        g_funcId2StringMapCacheHit++;
        //g_debugLogger << "CacheHit: #" << g_funcId2StringMapCacheHit << endl;
		return S_OK;
    }
	
	ZeroMemory(szFunction, sizeof(szFunction));
	char className[NAME_BUFFER_SIZE];
	ZeroMemory(className, sizeof(className));
	hr = g_info2->GetTokenAndMetaDataFromFunction(funcId, IID_IMetaDataImport, (LPUNKNOWN *) &metadata, &methodToken);
	if (hr != S_OK)
		return S_OK;
	//unsigned int moduleId = (fullClassId & 0xffff0000) >> 16;
	hr = metadata->QueryInterface(IID_IMetaDataImport2, (void**) &metadata2);
	ClassID classId;
	ModuleID moduleId;
	mdToken token;
	ULONG32 retrieved = 0;
	hr = g_info2->GetFunctionInfo2(funcId, frameInfo, &classId, &moduleId, &token, 0, &retrieved, NULL);
	ClassID parentClassId;
	ULONG32 dummy = 0;
	hr = g_info2->GetClassIDInfo2(classId, &moduleId, &classDef, &parentClassId, 0, &dummy, NULL);
	std::wstring symbolClassName;

	if (hr == S_OK)
	{
		mdToken extends;
		hr = metadata->GetTypeDefProps(classDef, szClass, NAME_BUFFER_SIZE, &classLength, 0, &extends);
		if (hr == S_OK)
		{

		}
		else
		{
            g_debugLogger << "Error: GetTypeDefProps() failed hr = 0x" << hex << hr << endl;
		}
	}

	if (hr == S_OK)
	{
		hr = metadata->GetMethodProps(methodToken,
			&typeDefToken, szFunction,
			NAME_BUFFER_SIZE, &cchMethod,
			NULL, &sigBlob, &sigBlobBytesCount, NULL, NULL);
		DWORD64 dummy64;
		TCHAR moduleNameW[200];

		moduleNameW[0] = 0;
		
		g_info2->GetModuleInfo(moduleId, (LPCBYTE*) &dummy64, sizeof(moduleNameW), (ULONG*)NULL, moduleNameW, NULL);
		std::wstring moduleName = std::wstring(moduleNameW);
		int lastIndex = moduleName.find_last_of(L"\\");
		lastIndex++;
		moduleName = moduleName.substr(lastIndex);

		if (hr == S_OK)
		{
			StackWalkFrame* newFrame = new StackWalkFrame();
			newFrame->IsManaged = true;
			newFrame->IsPartial = false;
			newFrame->FuncId = funcId;
			newFrame->EIP = contextData->Eip;
			newFrame->EBP = contextData->Ebp;
			newFrame->ESP = contextData->Esp;
            std::wstring symbolNameWide = moduleName + std::wstring(L"!") + std::wstring(szClass) + std::wstring(L".") + std::wstring(szFunction);            
            newFrame->SymbolName = ConvertStlString(symbolNameWide);
            g_funcId2StringMap[funcId] = newFrame->SymbolName;
			g_frames.push_back(*newFrame);
		}
	}
	return S_OK;
}

void CallstackSampler::ExpandNativeFrames()
{
	if (g_frames.size() == 0)
		return;
	g_debugLoggerVerbose.WriteLine("*** Managed Stack ***");
	g_stackLogger.WriteLine("*** Managed Stack ***");
    g_debugLogger << "g_frames.size()==" << dec << g_frames.size() <<" g_nativeFrames.size()==" << g_nativeFrames.size() << endl;

	std::list<std::string> annotFrames;
	int max = g_frames.size();
	DWORD64 lastEIP = 0;
	for(std::list<StackWalkFrame>::iterator frame_iter = g_frames.begin(); frame_iter != g_frames.end();)
	{
		StackWalkFrame& frame = *frame_iter;
		frame_iter++;
		if (frame.IsManaged)
		{			
            g_debugLoggerVerbose << frame.SymbolName.c_str();
            g_debugLoggerVerbose << "(UnknownLine) : 0x" << hex << frame.EIP << endl;
            g_stackLogger << frame.SymbolName.c_str();
            g_stackLogger << "(UnknownLine) : 0x" << hex << frame.EIP << endl;
			int skip = 1;
			DWORD64 adjustedIP = g_nativeStackwalker.GetNextEIP(frame.EIP, frame.EBP, frame.ESP, skip++);
			while ((adjustedIP & 0xFF000000) == 0)
			{
				adjustedIP = g_nativeStackwalker.GetNextEIP(frame.EIP, frame.EBP, frame.ESP, skip++);
			}
			DWORD64 startingIP = frame.EIP;
			lastEIP = startingIP /*adjustedIP*/;
            g_debugLoggerVerbose << "starting IP = 0x" << hex << startingIP << endl;
            g_debugLoggerVerbose << "adjusted IP = 0x" << hex << adjustedIP << endl;
            g_debugLoggerVerbose << "last     IP = 0x" << hex << lastEIP << endl;
		}
		else
		{
			if (lastEIP == 0)
				continue;
			bool found = false;
			int skip = 1;
			DWORD64 adjustedIP = g_nativeStackwalker.GetNextEIP(frame.EIP, frame.EBP, frame.ESP, skip++);
			while ((adjustedIP & 0xFF000000) == 0)
			{
				adjustedIP = g_nativeStackwalker.GetNextEIP(frame.EIP, frame.EBP, frame.ESP, skip++);
			}
			DWORD64 startingIP = frame.EIP;
            g_debugLoggerVerbose << "Unmanaged frames" << endl;
            g_debugLoggerVerbose << "starting IP = 0x" << hex << startingIP << endl;
            g_debugLoggerVerbose << "adjusted IP = 0x" << hex << adjustedIP << endl;
            g_debugLoggerVerbose << "last     IP = 0x" << hex << lastEIP << endl;

			if (lastEIP != 0)
			{
				frame.EIP = adjustedIP;
			}
			DWORD64 nextEIP = 0;

			if (frame_iter != g_frames.end())
			{
				StackWalkFrame& nextFrame = *frame_iter;
				nextEIP = g_nativeStackwalker.GetNextEIP(nextFrame.EIP, nextFrame.EBP, nextFrame.ESP, skip++);
                g_debugLoggerVerbose << "Next     IP = 0x" << hex << nextEIP << endl;
			}
			for(std::list<DWORD64>::iterator nativeFrame_iter = g_nativeFrames.begin(); nativeFrame_iter != g_nativeFrames.end(); nativeFrame_iter++)
			{
				DWORD64 nativeFrame = *nativeFrame_iter;
				if (nativeFrame == frame.EIP)
				{
					g_debugLoggerVerbose.WriteLine("EIP Found");
					found = true;
				}
				if (found)
				{
					std::string symbolName;
					if (g_nativeStackwalker.AnnotateStack(nativeFrame, symbolName) != S_OK)
					{
						g_debugLoggerVerbose.WriteLine(symbolName.c_str());						
						break;
					}
					g_debugLoggerVerbose.WriteLine(symbolName.c_str());
					g_stackLogger.WriteLine(symbolName.c_str());
					if (nativeFrame == nextEIP)
					{
						g_debugLoggerVerbose.WriteLine("LastEIP found");
						break;
					}
				}
			}
			if (found == false)
			{
				g_debugLoggerVerbose.WriteLine("EIP Not Found");
			}
			lastEIP = 0;
		}
	}
	g_debugLoggerVerbose.WriteLine("");
	g_stackLogger.WriteLine("");
	g_frames.clear();
	g_nativeFrames.clear();
}

HRESULT CallstackSampler::GetStackTrace(ICorProfilerInfo2* info2, ThreadID managedThreadId, DWORD nativeThreadId)
{
	if (info2 == nullptr)
		throw "CorProfilerInfo2 not set";
	if (nativeThreadId == 0)
		throw "ThreadId not set";

	g_info2 = info2;

    HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION | THREAD_GET_CONTEXT, FALSE, nativeThreadId);
    if(hThread == NULL)
    {
        g_debugLogger << "OpenThread failed with 0x"<< hex << GetLastError() << endl;		
        return S_FALSE;
    }

	HRESULT hr = SuspendThread(hThread);
	if (FAILED(hr))
	{
		g_debugLogger << "SuspendThread failed with 0x" << hex << hr << endl;	
		return S_FALSE;
	}
	
	CONTEXT context = {0};
	context.ContextFlags = CONTEXT_FULL;
	if (GetThreadContext(hThread, &context))
	{
		g_nativeStackwalker.CurrentThread = hThread;
		g_nativeFrames = g_nativeStackwalker.WalkStack2(context);
		g_frames.clear();
		if (managedThreadId == 0)
		{
			g_debugLoggerVerbose.WriteLine("---   Native Stack   ---");
			g_stackLogger.WriteLine("***   Native Stack   ***");
			std::list<std::string> annotFrames;
			hr = g_nativeStackwalker.AnnotateStack(g_nativeFrames, annotFrames);
			for each(std::string strFunc in annotFrames)
			{
				g_debugLoggerVerbose.WriteLine(strFunc.c_str());
				g_stackLogger.WriteLine(strFunc.c_str());
			}
			g_nativeFrames.clear();
			g_debugLoggerVerbose.WriteLine("");
			g_stackLogger.WriteLine("");
		}
		else
		{
			std::vector<FunctionID>* functions=new std::vector<FunctionID>();
			g_debugLoggerVerbose.WriteLine("---   DoStackSnapShot   ---");
		
			g_nativeStackwalker.CurrentThread = hThread;
			DWORD64 rip = context.Eip;
			hr = info2->DoStackSnapshot(managedThreadId, 
				MyDoStackSnapshotCallback, 
				COR_PRF_SNAPSHOT_REGISTER_CONTEXT,
				&functions,
				(BYTE*) &context,
				sizeof(CONTEXT));
			rip = context.Eip;
			ExpandNativeFrames();
			if (FAILED(hr))
			{
				goto cleanup;
            }
		}
	}
	else
	{
		g_debugLogger.WriteLine("GetThreadContextFailed");
	}

cleanup:
	if (hThread != NULL)
	{
		ResumeThread(hThread);
		CloseHandle(hThread);
	}
	return S_OK;
}
