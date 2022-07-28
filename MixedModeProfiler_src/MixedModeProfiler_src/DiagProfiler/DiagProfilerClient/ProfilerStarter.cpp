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

#include "ProfilerStarter.h"
#include <Windows.h>
#include <metahost.h>

#include <cor.h>
#include <corprof.h>

#include <list>
#include <string>
#include <iostream>
#include <string>
#include <sstream>
#include <tchar.h>
#include <strsafe.h>

#include "ClrHelper.h"

using namespace std;

static std::string ConvertStlString(std::wstring& text)
{    
    const std::string narrow(text.begin(), text.end());
    return narrow;
}

ProfilerStarter::ProfilerStarter()
{
	ZeroAll();
}

ProfilerStarter::~ProfilerStarter()
{
	ReleaseAll();
}

void ProfilerStarter::ReleaseAll()
{
	if (m_metahost != nullptr)
		m_metahost->Release();	
	if (m_runtimeInfo != nullptr)
		m_runtimeInfo->Release();
	if (m_clrProfiling != nullptr)
		m_clrProfiling->Release();
	ZeroAll();
}

void ProfilerStarter::ZeroAll()
{
	m_metahost = nullptr;
	m_clrProfiling = nullptr;
	m_runtimeInfo = nullptr;
	m_isAttached = false;
	m_processId = 0;
}	

HRESULT ProfilerStarter::Initialize()
{
	std::wstring version;
	if (!ClrHelper::GetV4RunTime(version))
		return E_FAIL;

	HRESULT hr = S_OK;
	do
	{
		hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID*)&m_metahost);
		if (hr != S_OK) break;
		hr = m_metahost->GetRuntime(version.c_str(), IID_ICLRRuntimeInfo, (LPVOID*)&m_runtimeInfo);
		if (hr != S_OK) break;
		hr = m_runtimeInfo->GetInterface(CLSID_CLRProfiling, IID_ICLRProfiling, (LPVOID *)&m_clrProfiling);
		if (hr != S_OK) break;
		return S_OK;
	} while (false);

	// We end up here only if we have errors
	ReleaseAll();
	return hr;
}	

DWORD ProfilerStarter::GetProcessId()
{
	return m_processId;
}

BOOL ProfilerStarter::StartProcess(const WCHAR* exePath)
{
	return StartProcess(exePath, NULL);
}

BOOL ProfilerStarter::StartProcess(const WCHAR* exePath, std::list<std::wstring>& environmentVars)
{	
	std::wstring envString;
	WCHAR separator[] = L"\0";
	for(std::list<std::wstring>::iterator iter = environmentVars.begin();iter != environmentVars.end();iter++)
	{
		std::wstring envText = *iter;
		envString += envText + std::wstring(separator, 1);
	}
	envString += std::wstring(separator, 1);
	WCHAR* envPtrTemp = const_cast<WCHAR*>(envString.c_str());
	LPVOID envPtr = reinterpret_cast<LPVOID>(envPtrTemp);
	return this->StartProcess(exePath, envPtr);
}

BOOL ProfilerStarter::StartProcess(const WCHAR* exePath, LPVOID environmentVars)
{

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	BOOL created = ::CreateProcessW(NULL // Application Name
		, const_cast<LPWSTR>(exePath) // CommandLine
		, NULL // Process Attributes
		, NULL // Thread Attributes
		, FALSE // Inherit Handles
		, CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT // Creation Flags
		, environmentVars // Environment
		, NULL // Current Directory
		, &si // Startup info
		, &pi);

	if (!created)
		return FALSE;
	m_processId = pi.dwProcessId;
	return m_processId != 0;
}

HRESULT ProfilerStarter::AttachCustomProfiler()
{
	return AttachCustomProfiler(5000);
}

HRESULT ProfilerStarter::AttachCustomProfiler(DWORD timeout, DWORD pId, std::list<std::wstring>& environmentVars)
{
	if (m_isAttached)
	{
		printf("Already attached\n");
		return E_FAIL;
	}
	if (pId == 0)
	{
		printf("processId == 0\n");
		return E_FAIL;
	}
	m_processId = pId;

    for (std::list<std::wstring>::iterator iter = environmentVars.begin(); iter != environmentVars.end(); iter++)
    {
        std::wstring envVarWide = *iter;
        std::string envVar = ConvertStlString(envVarWide);
        _putenv(envVar.c_str());
    }

	return AttachCustomProfiler(timeout);
}

HRESULT ProfilerStarter::AttachCustomProfiler(DWORD timeout)
{
	WCHAR buffer[500];
	DWORD bufferSize = sizeof(buffer)*sizeof(WCHAR);
	DWORD len = GetCurrentDirectory(bufferSize, buffer);
	if ((len > bufferSize) || (len == 0))
		return E_FAIL;

	std::wstring fullPath = std::wstring(buffer) + std::wstring(L"\\") + std::wstring(L"DiagProfiler.dll");

	if (m_processId == 0)
	{
		cout << "processId == 0" << endl;
		return E_FAIL;
	}
	if (m_isAttached)
	{
		cout << "Already attached" << endl;
		return E_FAIL;
	}
	HRESULT hr = Initialize();
	if (FAILED(hr))
	{
		cout << "Initialize() failed" << endl;
		return FALSE;
	}
		
	LPVOID pvClientData = NULL;
	DWORD cbClientData = 0;
	CLSID clsidProfiler;
	hr = CLSIDFromString(L"{C6DBEE4B-017D-43AC-8689-3B107A6104EF}", (LPCLSID)&clsidProfiler);
	hr = m_clrProfiling->AttachProfiler(m_processId,
									timeout, // ms max
									&clsidProfiler,
									//L"C:\\Users\\mattias.hogstrom\\Documents\\visual studio 2010\\Projects\\DiagProfiler\\Debug\\DiagProfiler.dll",
									fullPath.c_str(),
									pvClientData,
									cbClientData);
		
	m_isAttached = hr == S_OK;
    std::string path2Profiler = ConvertStlString(fullPath);
    int pId = static_cast<int>(m_processId);

  	ReleaseAll();


    cout << "AttachProfiler(" << pId << ", \"" << path2Profiler.c_str() << "\") : ";
	switch(hr)
	{
	case S_OK:
        cout << "S_OK" << endl;
		break;
	case CORPROF_E_PROFILER_ALREADY_ACTIVE:
		cout << "CORPROF_E_PROFILER_ALREADY_ACTIVE" << endl;
		break;
	case CORPROF_E_PROFILEE_INCOMPATIBLE_WITH_TRIGGER:
		cout << "CORPROF_E_PROFILEE_INCOMPATIBLE_WITH_TRIGGER" << endl;
		break;
	case CORPROF_E_PROFILER_NOT_ATTACHABLE:
		cout << "CORPROF_E_PROFILER_NOT_ATTACHABLE" << endl;
		break;
	case E_INVALIDARG:
		cout << "E_INVALIDARG" << endl;
		break;
	case ERROR_TIMEOUT:
		cout << "ERROR_TIMEOUT" << endl;
		break;
	case ERROR_FILE_NOT_FOUND:
		cout << "Process does not exist" << endl;
		break;
	case CORPROF_E_IPC_FAILED:
		cout << "CORPROF_E_IPC_FAILED" << endl;
		break;
	case E_FAIL:
		cout << "E_FAIL" << endl;
		break;
	default:
		cout << "Unknown Error" << endl;
		break;
	}

	return hr;
}
