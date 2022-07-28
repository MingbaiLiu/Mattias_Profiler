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
#include <metahost.h>
#include <list>
#include <iostream>
class ProfilerStarter
{
public:
	ProfilerStarter();
	~ProfilerStarter();
	BOOL StartProcess(const WCHAR* exePath);
	BOOL StartProcess(const WCHAR* exePath, LPVOID environmentVars);
	BOOL StartProcess(const WCHAR* exePath, std::list<std::wstring>& environmentVars);
	HRESULT AttachCustomProfiler();
	HRESULT AttachCustomProfiler(DWORD timeout);
	HRESULT AttachCustomProfiler(DWORD timeout, DWORD pId, std::list<std::wstring>& environmentVars);
	DWORD GetProcessId();
private:
	HRESULT Initialize();	
	void ReleaseAll();
	void ZeroAll();
private:
	// the follow info is more importance, those will init ICLRMetaHost, ICLRProfiling and ICLRRuntimeInfo
	ICLRMetaHost* m_metahost; 
	ICLRProfiling* m_clrProfiling;
	ICLRRuntimeInfo* m_runtimeInfo;
	bool m_isAttached;
	DWORD m_processId;
};