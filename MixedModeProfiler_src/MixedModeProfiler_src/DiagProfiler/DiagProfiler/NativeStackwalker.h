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

#include <windows.h>
#include <list>
#include <string>

class NativeStackwalker
{
public:
		NativeStackwalker(void);
		~NativeStackwalker(void);
        void SetSymbolPath(std::string path);
        std::string GetSymbolPath();
        std::string GetEnvSymbolPath();
		std::list<DWORD64> WalkFrame(const CONTEXT& context)
		{
			return WalkFrames(context, 0, -1);
		}
		bool ForceLoad(DWORD64 addr);
		void LoadPdb(std::string& fileName, DWORD64 baseAddr);
		void DisplayDebugData();

        static HRESULT ListProcessThreads(DWORD dwOwnerPID, std::list<DWORD>& threadList);

		DWORD64 GetNextEIP(DWORD64 EIP, DWORD64 EBP, DWORD64 ESP, int skip);
		std::list<DWORD64> WalkFrames(const CONTEXT& context, int skip, int max);
		std::list<DWORD64> WalkStack(int skipFrame, HANDLE processHandle, HANDLE threadHandle);
		std::list<DWORD64> WalkStack2(const CONTEXT& context);
		HRESULT AnnotateStack(std::list<DWORD64>& stackFrames, std::list<std::string>& annotatedStackFrames);
		HRESULT AnnotateStack(DWORD64 addr, std::string& result);
		HANDLE CurrentThread;
private:
		bool m_sym_is_initialized;
		HANDLE m_currentProcess;
		HANDLE m_currentThread;
        std::string m_symbolPath;
};
