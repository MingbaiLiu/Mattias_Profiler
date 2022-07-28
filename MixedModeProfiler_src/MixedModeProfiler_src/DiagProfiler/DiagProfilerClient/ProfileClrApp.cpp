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

#include "Stdafx.h"

#include "ProfileClrApp.h"
#include "ProfilerStarter.h"
#include "ProfilerCommandClient.h"
#include "ClrHelper.h"

#include <vcclr.h>
#include <iostream>
#include <sstream>

using namespace std;

static std::string ConvertStlString(std::wstring& text)
{    
    const std::string narrow(text.begin(), text.end());
    return narrow;
}

ProfileClrApp::ProfileClrApp()
{
	m_pId = 0;
	m_client = new ProfilerCommandClient();
	m_profilerStarter = new ProfilerStarter();
}

ProfileClrApp::~ProfileClrApp()
{
	if (m_client != nullptr)
	{
		delete m_client;
		m_client = nullptr;
	}
	if (m_profilerStarter)
	{
		delete m_profilerStarter;
		m_profilerStarter = nullptr;
	}
}

std::wstring StringToWString(String^ clrString)
{
   pin_ptr<const wchar_t> wch = PtrToStringChars(clrString);
   return std::wstring(wch);
}

bool ProfileClrApp::ClrRuntimeIsV4(String^ exePath)
{
	std::wstring pathToExe = StringToWString(exePath);
	std::wstring clrVersion = ClrHelper::GetClrVersion(pathToExe.c_str());
	return ClrHelper::ParseClrVersion(clrVersion.c_str()) == V4;
}

bool ProfileClrApp::StartProcess(String^ exePath, String^ stackOutputPath)
{
    return this->StartProcess(exePath, stackOutputPath, String::Empty);
}

bool ProfileClrApp::StartProcess(String^ exePath, String^ stackOutputPath, String^ debugOutputPath)
{
    if (m_pId != 0)
    	return false;

    m_symbolPath = System::IO::Path::GetDirectoryName(exePath);
    String^ assemblyDir = System::IO::Path::GetDirectoryName(System::Reflection::Assembly::GetExecutingAssembly()->GetName()->CodeBase);
    String^ profilerDir = assemblyDir;
    String^ prefix = "file:\\";
    if (assemblyDir->StartsWith(prefix))
        profilerDir = assemblyDir->Substring(prefix->Length);

    m_profilerPath = System::IO::Path::Combine(profilerDir,  "DiagProfiler.dll");
    m_exePath = exePath;
	std::wstring pathToExe = StringToWString(exePath);
    m_stackOutputPath = stackOutputPath;
    std::wstring pathToStackOutput = StringToWString(stackOutputPath);
    m_debugOutputPath = debugOutputPath;
    std::wstring pathToDebugOutput = StringToWString(debugOutputPath);
    std::wstring pathToProfiler = StringToWString(m_profilerPath);
    std::wstring pathToSymbolDir = StringToWString(m_symbolPath);

	std::list<std::wstring> environment;
    
    std::wstring symbolPath = std::wstring(L"DIAG_PRF_SYMBOLPATH=") + pathToSymbolDir;
    environment.push_back(symbolPath);
	
    environment.push_back(std::wstring(L"DIAG_PRF_RUN_WITH_GUI=1"));
	environment.push_back(std::wstring(L"DIAG_PRF_ONLY_MANAGED_THREADS=1"));

    std::wstring stacktrace = std::wstring(L"DIAG_PRF_STACKTRACE=") + pathToStackOutput;
	environment.push_back(stacktrace);
    
    std::wstring debugtrace = std::wstring(L"DIAG_PRF_DEBUGTRACE=") + pathToDebugOutput;
	environment.push_back(debugtrace);

	environment.push_back(std::wstring(L"COR_ENABLE_PROFILING=1"));
	environment.push_back(std::wstring(L"COR_PROFILER={C6DBEE4B-017D-43AC-8689-3B107A6104EF}"));
    std::wstring cor_profiler_path = std::wstring(L"COR_PROFILER_PATH=") + pathToProfiler;
	environment.push_back(cor_profiler_path);

    std::wstring clr4Version;
    if (!ClrHelper::GetV4RunTime(clr4Version))
    {
        cout << "Cannot find clr v4 installed" << endl;
        return false;
    }
    std::wstring forceVersion = std::wstring(L"COMPLUS_Version=") + clr4Version;
	environment.push_back(forceVersion);
    
    environment.push_back(std::wstring(L"COMPLUS_Version=v4.0.30319"));	
	//environment.push_back(std::wstring(L"COMPLUS_Version=v2.0.50727"));

	BOOL created = m_profilerStarter->StartProcess(pathToExe.c_str(), environment);
	if (!created)
	{
        cout << "Failed to create process" << endl;
		return false;
	}
	cout << "Process created" << endl;
    cout << "Forcing version " << ConvertStlString(forceVersion).c_str() << endl;
	cout << "Waiting 1000 ms for Process to startup in order to attach to it" << endl;
	Sleep(1000);
	m_pId = m_profilerStarter->GetProcessId();
	return true;
}

bool ProfileClrApp::AttachProfiler(DWORD pId)
{
	m_pId = pId;
	return AttachProfiler() != 0;
}

int ProfileClrApp::PId()
{
	return m_pId;
}

bool ProfileClrApp::ConnectGUI()
{
	if (m_pId == 0)
		return false;
	HRESULT hr = m_client->ConnectToProfiler(m_pId, 3000);
	if (SUCCEEDED(hr))
	{
		return true;
	}
	return false;
}

bool ProfileClrApp::AttachProfiler()
{
	if (m_pId == 0)
		return false;

    WCHAR buffer[500];
	DWORD bufferSize = sizeof(buffer)/sizeof(WCHAR);
	DWORD len = GetCurrentDirectory(bufferSize, buffer);
	if ((len > bufferSize) || (len == 0))
		return false;

    std::wostringstream o;
    o << buffer;
    o << L"\\";
    o << L"stacktrace_";
    o << m_pId;
    o << L".txt";
    o.clear();
    std::wstring pathToStackOutput = o.str();
    
    o << buffer;
    o << L"\\";
    o << L"debugtrace_";
    o << m_pId;
    o << L".txt";
    std::wstring pathToDebugOutput = o.str();
    
    std::list<std::wstring> environment;
	environment.push_back(std::wstring(L"DIAG_PRF_RUN_WITH_GUI=1"));
	environment.push_back(std::wstring(L"DIAG_PRF_ONLY_MANAGED_THREADS=0"));
    std::wstring stacktrace = std::wstring(L"DIAG_PRF_STACKTRACE=") + pathToStackOutput;
	environment.push_back(stacktrace);
    
    std::wstring debugtrace = std::wstring(L"DIAG_PRF_DEBUGTRACE=") + pathToDebugOutput;
	environment.push_back(debugtrace);


	HRESULT hr = m_profilerStarter->AttachCustomProfiler(2000, m_pId, environment);
	if (SUCCEEDED(hr))
	{
		return true;
	}
	return true;
}

bool ProfileClrApp::StartSampling()
{
	HRESULT hr = m_client->StartSampling();
	if (hr != S_OK)
	{
		return false;
	}	
	return true;
}

bool ProfileClrApp::SetStackLoggerFile(String^ stackOutputPath)
{
    std::wstring wideFileName =  StringToWString(stackOutputPath);
    std::string fileName = ConvertStlString(wideFileName);
    HRESULT hr = m_client->SetStackLoggerFile(fileName);
    return hr == S_OK;
}

bool ProfileClrApp::SetMaxSamples(int maxNoSamples)
{
    HRESULT hr = m_client->SetMaxSamples(maxNoSamples);
    return hr == S_OK;
}

bool ProfileClrApp::SetSampleRate(int rateMs)
{
    HRESULT hr = m_client->SetSampleRate(rateMs);
    return hr == S_OK;
}

int ProfileClrApp::GetSampleRate()
{
	int rate = 0;
	HRESULT hr = m_client->GetSampleRate(rate);
	if (hr != S_OK)
	{
		return 0;
	}	
	return rate;
}

int ProfileClrApp::GetSampleCount()
{
	int noSamples = 0;
	HRESULT hr = m_client->GetSampleCount(noSamples);
	if (hr != S_OK)
	{
		return 0;
	}	
	return noSamples;
}

bool ProfileClrApp::StopSampling()
{
	HRESULT hr = m_client->StopSampling();
	if (hr != S_OK)
	{
		return false;
	}
	return true;
}
