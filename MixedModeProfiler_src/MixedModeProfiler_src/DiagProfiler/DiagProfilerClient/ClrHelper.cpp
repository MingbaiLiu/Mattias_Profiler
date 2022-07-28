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

#include "ClrHelper.h"

#pragma comment(lib, "mscoree.lib")
#include <mscoree.h>
#include <string>
#include <metahost.h>

ClrHelper::ClrHelper()
{
}

ClrHelper::~ClrHelper()
{
}

std::wstring ClrHelper::GetClrVersion(const wchar_t* exePath)
{
	const int MAXBUFFER_LEN = 20;
	WCHAR runtimeVersion[MAXBUFFER_LEN];
	DWORD versionStringLength = 0;
		
	HRESULT hr = GetRequestedRuntimeVersion(const_cast<LPWSTR>(exePath), runtimeVersion, MAXBUFFER_LEN, &versionStringLength);
	if (hr != S_OK)
		return std::wstring();
	else
		return std::wstring(runtimeVersion);
}

ClrVersion ClrHelper::ParseClrVersion(const wchar_t* version)
{
	std::wstring clrVersion = std::wstring(version);	
	if (clrVersion.find(L"v1.") != std::string::npos)
		return V1;
	if (clrVersion.find(L"v2.") != std::string::npos)
		return V2;
	if (clrVersion.find(L"v4.") != std::string::npos)
		return V4;
	return Undefined;
}

BOOL ClrHelper::GetV4RunTime(std::wstring& clr4Runtime)
{
	std::list<std::wstring> installedRuntimes;
	HRESULT hr = ClrHelper::GetInstalledClrRuntimes(installedRuntimes);
	if (hr != 0)
		return FALSE;

	for each(std::wstring runtimeVersion in  installedRuntimes)
	{
		wprintf(L"%s\n", runtimeVersion.c_str());
		if (ClrHelper::ParseClrVersion(runtimeVersion.c_str()) == V4)
		{
			clr4Runtime = std::wstring(runtimeVersion.c_str());
			break;
		}
	}

	return clr4Runtime.size() != 0;
}

HRESULT ClrHelper::GetInstalledClrRuntimes(std::list<std::wstring>& clrRuntimeList)
{
	HRESULT hr = S_OK;		
	clrRuntimeList.clear();
	ICLRMetaHost* metahost = nullptr;
	hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID*)&metahost);
	if (FAILED(hr))
		return hr;

	IEnumUnknown* runtimeEnumerator = nullptr;
	hr = metahost->EnumerateInstalledRuntimes(&runtimeEnumerator);
	if (SUCCEEDED(hr))
	{
		WCHAR currentRuntime[50];
		DWORD bufferSize = ARRAYSIZE(currentRuntime);
		IUnknown* runtime = nullptr;
		while (runtimeEnumerator->Next(1, &runtime, NULL) == S_OK)
		{
			ICLRRuntimeInfo* runtimeInfo = nullptr;
			hr = runtime->QueryInterface(IID_PPV_ARGS(&runtimeInfo));
			if (SUCCEEDED(hr))
			{
				hr = runtimeInfo->GetVersionString(currentRuntime, &bufferSize);
				if (SUCCEEDED(hr))
				{
					clrRuntimeList.push_back(std::wstring(currentRuntime));
				}
				runtimeInfo->Release();
			}
			runtime->Release();
		}
		runtimeEnumerator->Release();
		hr = S_OK;
	}
	metahost->Release();
	return hr;
}
