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

#include "stdafx.h"

#include <windows.h>
#include <objbase.h>

#include <IDiagProfiler_h.h>
#include <IDiagProfiler_i.c>

#include <objbase.h>

#include <stdio.h>
#include <string>
#include <cor.h>
#include <corprof.h>
#include <corpub.h>
#include <cordebug.h>

#include <list>
#include <string>
#include <iostream>
#include <string>
#include <sstream>
#include <tchar.h>
#include <strsafe.h>
#include <metahost.h>
#include <cordebug.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "CorGuids.lib") 

#pragma comment(lib, "mscoree.lib")

using namespace std;
using namespace System;

static std::string ConvertStlString(std::wstring& text)
{    
    const std::string narrow(text.begin(), text.end());
    return narrow;
}

static std::wstring ConvertStlString(std::string& text)
{
    const std::wstring wide(text.begin(), text.end());
    return wide;
} 

static std::string GetCurrentDirectoryAsStlString()
{
   	CHAR buffer[500];
	DWORD bufferSize = sizeof(buffer)*sizeof(WCHAR);
	DWORD len = GetCurrentDirectoryA(bufferSize, buffer);
	if ((len > bufferSize) || (len == 0))
		return std::string("");
    return std::string(buffer);
}

int _tmain(int argc, _TCHAR* argv[])
{
    std::string currentDir = GetCurrentDirectoryAsStlString();

	ULONG pId = 0;
	if (argc == 2)
	{
		std::basic_string<TCHAR> str = std::basic_string<TCHAR>(argv[1]);
		pId = _wtol(argv[1]);
		HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pId);
		if (processHandle == 0)
		{
            cout << "Process Id " << pId << "doesn't not exist or access is not granted" << endl;
			return -1;
		}
		CloseHandle(processHandle);
	}

	ProfileClrApp^ profiler = gcnew ProfileClrApp(); // init profile App

	BOOL profilerAttached = FALSE;
	BOOL profilerConnected = FALSE;
    
    const int maxSamples = 100;
    const int samplingRate = 25;

	if (pId != 0)
	{
		profilerAttached = profiler->AttachProfiler(pId);
		if (profilerAttached)
		{
			profilerConnected = profiler->ConnectGUI();
            if (!profilerConnected)
            {
                return -1;
            }
            if (!profiler->SetSampleRate(samplingRate))
            {
                cout << "Failed to set max samples" << endl;
            }
            else
            {
                cout << "Sampling Rate: " << samplingRate << endl;
            }
            if (!profiler->SetMaxSamples(maxSamples))
            {
                cout << "Failed to set max samples" << endl;
            }
            else
            {
                cout << "SetMaxSamples: " << maxSamples << endl;
            }

            std::ostringstream o;
            o << currentDir.c_str();
            o << "\\diagstack_" << pId << ".txt";
            std::string stackFileName = o.str();
            String^ fileName = gcnew String(stackFileName.c_str());
            if (!profiler->SetStackLoggerFile(fileName))
            {
                cout << "Failed to set stack log file" << endl;
                profiler->StopSampling();
            }
            else
            {
                cout << "StackLogFile=" << stackFileName.c_str() << endl;
            }
		}
	}
	else
	{		
        std::string executablePath = currentDir + std::string("\\CppCliApp.exe");
        std::string stackOutputPath = currentDir + std::string("\\CppCliApp_trace.txt");
        std::string debugOutputPath = currentDir + std::string("\\CppCliApp_debug.txt");

		String^ exePath = gcnew String(executablePath.c_str());
        String^ stackPath = gcnew String(stackOutputPath.c_str());
        String^ debugPath = gcnew String(debugOutputPath.c_str());

        cout << "Trying to start sample app: " << executablePath.c_str() << endl;
		if (!profiler->StartProcess(exePath, stackPath, debugPath))
		{
            cout << "Failed to start process" << endl;
			return -1;
		}
		profilerConnected = profiler->ConnectGUI();
	}	
	
	if (!profilerConnected)
	{
		cout << "Failed to attach to process" << endl;
		return -1;
	}
    int rate = profiler->GetSampleRate();
    cout << "Samplerate: " << rate << " ms" << endl; 

    if (!profiler->StartSampling())
  	{
		cout << "Failed to start sampling" << endl;
        return -1;
	}

    cout << "Started Sampling" << endl;
    cout << "Will run #" << maxSamples << " samples" << endl;
    
    int expectedTime = samplingRate * maxSamples;
    Sleep(1000 + expectedTime);
    profiler->StopSampling();
    cout << "Stopped Sampling" << endl;
    int count = profiler->GetSampleCount();

    // We might get less samples if the OnSample consumes more time than the interval
    // There are three ways to remedy this
    // 1. Use DIAG_PRF_ONLY_MANAGED_THREADS=1
    //    At this point, it only works with StartProcess, not ProfilerAttach
    // 2. Optimize the OnSample stackwalk
    // 3. Increase the sample interval
    // 4. Tell the profiler specifically which threads we are interested in
    //    Currently not supported. But could be implmented through the CommandServer

    cout << "Got #" << count << " samples" << endl;

    cout << flush;
	system("pause");

	return 0;
}

