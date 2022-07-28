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

#include <cordebug.h>
#include <metahost.h>
#include <list>
#include <string>
#include <map>

class CallstackSampler
{
private:
	CallstackSampler(CallstackSampler& copy) {}
public:
    CallstackSampler() {}
    ~CallstackSampler() {}
	HRESULT GetStackTrace(ICorProfilerInfo2* info2, ThreadID managedThreadId, DWORD nativeThreadId);
private:
    void ExpandNativeFrames();
    static HRESULT __stdcall MyDoStackSnapshotCallback (
                    FunctionID funcId,
                    UINT_PTR ip,
                    COR_PRF_FRAME_INFO frameInfo,
                    ULONG32 contextSize,    
                    BYTE context[],
                    void *clientData
                    );
};