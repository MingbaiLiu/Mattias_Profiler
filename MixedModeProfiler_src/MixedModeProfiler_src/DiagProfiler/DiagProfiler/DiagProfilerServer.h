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
#include "ProfilerCommandServer.h"

struct IDiagProfiler;
class FileLogger;

class DiagProfilerServer
{
public:
    DiagProfilerServer();
    ~DiagProfilerServer();
    BOOL Start(IDiagProfiler* diagProfiler, DWORD timeout);
    DWORD GetThreadId() { return m_commandServer.GetThreadId(); }

private:
    IDiagProfiler* m_diagProfiler;
    ProfilerCommandServer m_commandServer;
};

