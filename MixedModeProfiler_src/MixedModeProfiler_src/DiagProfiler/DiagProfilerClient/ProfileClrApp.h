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
#include <string>
#include "ProfilerCommandClient.h"
#include "ProfilerStarter.h"

using namespace System;

public ref class ProfileClrApp
{
public:
	ProfileClrApp();
	~ProfileClrApp();

    bool StartProcess(String^ exePath, String^ stackOutputPath);
    bool StartProcess(String^ exePath, String^ stackOutputPath, String^ debugOutputPath); // start a process 
	bool AttachProfiler();
	bool AttachProfiler(DWORD pId);
	bool ConnectGUI();
	bool StartSampling();
	bool StopSampling();
	int GetSampleCount();
	int GetSampleRate();
    bool SetMaxSamples(int maxNoSamples);
	bool SetSampleRate(int rateMs);
    bool SetStackLoggerFile(String^ stackOutputPath);

	bool ClrRuntimeIsV4(String^ exePath);
	int PId();
	 
private:
	String^ m_exePath;
    String^ m_stackOutputPath;
    String^ m_debugOutputPath;
    String^ m_profilerPath;
    String^ m_symbolPath;
	DWORD m_pId;
	ProfilerCommandClient* m_client;
	ProfilerStarter* m_profilerStarter;
};
