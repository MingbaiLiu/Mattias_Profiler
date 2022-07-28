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

class ProfilerCommandClient
{
private:	
	HANDLE m_pipeToProfiler;

public:
	ProfilerCommandClient();
	~ProfilerCommandClient();
	
	HRESULT SendCommand(const char* command);
	HRESULT SendCommand(const char* command, std::string& commandResult);
	HRESULT StartSampling();
	HRESULT StopSampling();
	HRESULT GetSampleCount(int& noSamples);
	HRESULT SetMaxSamples(int noSamples);
	HRESULT SetSampleRate(int rate);
	HRESULT GetSampleRate(int& rate);
    HRESULT SetStackLoggerFile(std::string& stackOutputPath);


	HRESULT ConnectToProfiler(DWORD pId);
	HRESULT ConnectToProfiler(DWORD pId, DWORD timeout);
};

