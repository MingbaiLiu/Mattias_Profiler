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
#include <list>

enum ClrVersion
{
	Undefined = 0,
	V1 = 1,
	V2 = 2,
	V4 = 4
};

class ClrHelper
{
private:
	ClrHelper();
public:
	~ClrHelper();

	static std::wstring GetClrVersion(const wchar_t* exePath);
	static ClrVersion ParseClrVersion(const wchar_t* version);

	static BOOL GetV4RunTime(std::wstring& clr4Runtime);
	static HRESULT GetInstalledClrRuntimes(std::list<std::wstring>& clrRuntimeList);
};

