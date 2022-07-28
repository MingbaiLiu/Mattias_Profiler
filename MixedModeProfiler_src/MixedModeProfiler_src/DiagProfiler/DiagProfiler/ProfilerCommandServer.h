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
#include "PipeServerAsync.h"
#include "ProfilerCommand.h"
#include <string>
#include <map>


typedef std::map<std::string, ProfilerCommand*> TStrCmdMap;
typedef std::pair<std::string, ProfilerCommand*> TStrCmdPair;

class ProfilerCommandServer
{
	PipeServerAsync m_pipeServer;
	HANDLE m_threadHandle;
	DWORD m_threadId;
	BOOL m_stopping;
	HANDLE m_event;
	TStrCmdMap m_commandMap;

public:
	ProfilerCommandServer();
	~ProfilerCommandServer();
	BOOL IsRegistered(std::string& command);
	BOOL IsRegistered(const char* command);
	BOOL RegisterCommand(const char* name, PrfCmdFuncBoolStringVoid action);
	BOOL RegisterCommand(const char* name, PrfCmdFuncBoolStringString function);
	//BOOL RegisterCommand(const char* name, PrfCmdMethBoolStringVoid action);
	//BOOL RegisterCommand(const char* name, PrfCmdMethBoolStringString function);
    BOOL ExecuteCommand(std::string& name, std::string& result);
	BOOL Start(DWORD timeout);
	BOOL Stop();
	BOOL Run();
	BOOL IsRunning();
	BOOL Receive(std::string& message);
	BOOL Send(std::string& message);
	BOOL Pulse();
	DWORD GetThreadId();
};