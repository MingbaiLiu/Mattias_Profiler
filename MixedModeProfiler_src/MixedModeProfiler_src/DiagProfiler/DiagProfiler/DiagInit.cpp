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
#include "DiagInit.h"
#include "NativeStackwalker.h"
#include "DiagGlobals.h"
#include "PerformanceCounter.h"

using namespace std;

DiagInit::DiagInit()
{
    InitializeCriticalSection(&m_cs_debug);
    InitializeCriticalSection(&m_cs_stack);
    
    m_debugLogger.SetPrefix("[DiagProfiler] ");
    m_debugLogger.Echo2OutputDebug(true);

    m_debugLoggerVerbose.SetPrefix("[DiagProfiler+] ");
    // enable verbose output here
    m_debugLoggerVerbose.Echo2OutputDebug(false);

    // g_debugLogger works even if the file is not open
    // It can signal errors when the file cannot be opened
    g_debugLogger.WriteLine("DiagProfilerFactory::DiagProfilerFactory()");

    std::string envVerbosename = std::string("DIAG_PRF_VERBOSE");
    std::string debugenvVerbosename = GetEnvVar(envVerbosename);
    if (debugenvVerbosename.size() != 0)
    {
        m_debugLoggerVerbose.Open(debugenvVerbosename.c_str());
        if (!m_debugLoggerVerbose.IsOpen())
        {
            g_debugLogger << "DebugLogger - Failed to open file: " << debugenvVerbosename.c_str() << endl;
        }
    }

  	std::string envFilename = std::string("DIAG_PRF_DEBUGTRACE");
	std::string debugFilename = GetEnvVar(envFilename);
	if (debugFilename.size() != 0)
	{
		m_debugLogger.Open(debugFilename.c_str());
        if (!m_debugLogger.IsOpen())
        {
            g_debugLogger << "DebugLogger - Failed to open file: " << debugFilename.c_str() << endl;
        }
	}

    std::string envStacktrace = std::string("DIAG_PRF_STACKTRACE");
	std::string stackFilename = GetEnvVar(envStacktrace);
	if (stackFilename.size() != 0)
	{
		g_stackLogger.Open(stackFilename.c_str());
        if (!m_stackLogger.IsOpen())
        {
            g_debugLogger << "StackLogger - Failed to open file: " << debugFilename.c_str() << endl;
        }
	}
	else
	{
        g_debugLogger.WriteLine("StackLogger - A file must be specified");
        g_debugLogger.WriteLine("SET DIAG_PRF_STACKTRACE=<file>");
	}

    g_debugLogger.WriteLine("DiagProfilerFactory::DiagProfilerFactory()");

    PerformanceCounter perfCounter;
    g_debugLogger << "PERF - TicksPerSecond = " << dec << perfCounter.GetTicksPerSecond() << endl;

    m_nativeStackwalker = new NativeStackwalker();
}

DiagInit::~DiagInit()
{
    DeleteCriticalSection(&m_cs_debug);
    DeleteCriticalSection(&m_cs_stack);
    delete m_nativeStackwalker;
}
