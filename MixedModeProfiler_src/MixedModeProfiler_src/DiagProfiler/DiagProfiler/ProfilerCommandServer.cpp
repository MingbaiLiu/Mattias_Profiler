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

#include "ProfilerCommandServer.h"
#include <iostream>
#include <sstream>
#include <tchar.h>
#include <strsafe.h>
#include "DiagGlobals.h"

using namespace std;

ProfilerCommandServer::ProfilerCommandServer()
{
	m_threadId = 0;
	m_threadHandle = INVALID_HANDLE_VALUE;
	m_stopping = FALSE;
	m_event = CreateEvent(NULL, FALSE, FALSE, NULL);
}

BOOL ProfilerCommandServer::RegisterCommand(const char* name, PrfCmdFuncBoolStringVoid action)
{
	if (action == nullptr)
		return FALSE;
	if (IsRegistered(name))
		return FALSE;
	m_commandMap.insert(TStrCmdPair(std::string(name), new ProfilerCommand(name, action)));
	return TRUE;
}

BOOL ProfilerCommandServer::RegisterCommand(const char* name, PrfCmdFuncBoolStringString function)
{
	if (function == nullptr)
		return FALSE;
	if (IsRegistered(name))
		return FALSE;
	m_commandMap.insert(TStrCmdPair(std::string(name), new ProfilerCommand(name, function)));
	return TRUE;
}

BOOL ProfilerCommandServer::Pulse()
{
	return SetEvent(m_event);
}

DWORD ProfilerCommandServer::GetThreadId()
{
	return m_threadId;
}

ProfilerCommandServer::~ProfilerCommandServer()
{
	CloseHandle(m_event);
    m_event = INVALID_HANDLE_VALUE;
	this->Stop();
}

BOOL ProfilerCommandServer::IsRunning()
{
	if (m_threadId != 0)
	{
		HANDLE tempHandle = OpenThread(READ_CONTROL, FALSE, m_threadId);
		CloseHandle(tempHandle);
	}
	return TRUE;
}

BOOL ProfilerCommandServer::Send(std::string& message)
{
	if (m_pipeServer.SendBegin(message))
	{
		return m_pipeServer.WaitForIO(1000);
	}
	return FALSE;
}

BOOL ProfilerCommandServer::Receive(std::string& message)
{
	if (m_pipeServer.HasPendingIO())
		return FALSE;
	if (m_pipeServer.ReceiveBegin())
	{
		if (m_pipeServer.WaitForIO(INFINITE))
		{
			if (m_pipeServer.ReceiveEnd(message))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL ProfilerCommandServer::IsRegistered(std::string& command)
{
	TStrCmdMap::const_iterator iter = m_commandMap.find(command);
	return (iter != m_commandMap.end());
}

BOOL ProfilerCommandServer::IsRegistered(const char* command)
{
	TStrCmdMap::const_iterator iter = m_commandMap.find(command);
	return (iter != m_commandMap.end());
}

BOOL ProfilerCommandServer::ExecuteCommand(std::string& name, std::string& result)
{
	ProfilerCommand* command = nullptr;
	std::string::size_type requiresPars = name.find_first_of('=');
    
	std::string pars;
	int index = requiresPars;
    g_debugLogger << "ProfilerCommandServer::ExecuteCommand(" << name.c_str() << ")" << endl;

    std::string commandName;
	if (requiresPars == std::string::npos) // npos == bad length constant (-1)
	{
        g_debugLogger.WriteLine("Executing command without parameters");
        commandName = name;
	}
	else
	{
        g_debugLogger.WriteLine("Executing command with parameters");
       	g_debugLogger << "RequiresPars = " << index << endl;

        commandName = std::string(name, 0, index);
		int copyLength = name.size() - requiresPars - 1;
		pars = name.substr(index+1, copyLength);
        g_debugLogger << "Pars = " << pars.c_str() << endl;
	}

    if (!IsRegistered(commandName))
    {
        g_debugLogger << "ProfilerCommandServer::ExecuteCommand" <<endl;
        g_debugLogger << "IsRegistered(" << commandName.c_str() << ") == FALSE" << endl;
		return FALSE;
    }

    command = m_commandMap[commandName];
    
	if (command == nullptr)
		return FALSE;
	BOOL executeResult = FALSE;
	if (pars.length() != 0)
	{
		executeResult = command->Execute(result, pars);
	}
	else
	{
		executeResult = command->Execute(result);
	}
	return executeResult;
}

DWORD WINAPI Thread_Command_Server( LPVOID lpParam )
{
	static int messageCounter = 0;
	ProfilerCommandServer* instance = reinterpret_cast<ProfilerCommandServer*>(lpParam);
	std::string receiveMessage;
	while(TRUE)
	{
		if (instance->Receive(receiveMessage))
		{			
			messageCounter++;
            g_debugLogger << "Received = " << receiveMessage << std::endl;
            std::string outputResult;
			BOOL executeResult = instance->ExecuteCommand(receiveMessage, outputResult);
			g_debugLogger << "Executed = " << executeResult << std::endl;
			std::ostringstream o;
			o << "[" << messageCounter <<"]" <<"[" << executeResult <<"]";
			o << outputResult;
			std::string returnMessage = o.str();
			instance->Send(returnMessage);
		}
		else
		{
            g_debugLogger.WriteLine("Error: ProfilerCommandServer::Receive() failed");
            g_debugLogger.WriteLine("Terminating command server thread");
			instance->Pulse();
			break;
		}
	}
	return 0;
}

BOOL ProfilerCommandServer::Start(DWORD timeout)
{
    g_debugLogger.WriteLine("ProfilerCommandServer::Start");
	m_pipeServer.Start();
	BOOL connecting = m_pipeServer.ConnectToClientBegin();
	if (!connecting)
	{
        g_debugLogger.WriteLine("pipe is not connecting");
		return FALSE;
	}
	BOOL connected = m_pipeServer.ConnectToClientEnd(timeout);
	if (connected)
	{
        g_debugLogger.WriteLine("Client connected to server");
		m_threadId = 0;
		m_threadHandle = CreateThread(NULL, 0, Thread_Command_Server, this, 0, &m_threadId);
		if (m_threadHandle == 0)
		{
			this->Stop();
			return FALSE;
		}
	}
	return connected;
}

BOOL ProfilerCommandServer::Stop()
{
	if (!m_pipeServer.IsConnected())
		return FALSE;
	BOOL releaseWaiters = m_pipeServer.ReleaseWaiters();
	DWORD dwWait = WaitForSingleObject(m_threadHandle, INFINITE);
	if (dwWait == WAIT_OBJECT_0)
	{
		m_threadHandle = 0;
		m_threadId = 0;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL ProfilerCommandServer::Run()
{
	if (!m_pipeServer.IsConnected())
		return FALSE;
	if (m_threadHandle != INVALID_HANDLE_VALUE)
	{
		DWORD dwWait = WaitForSingleObjectEx(m_event, INFINITE, TRUE);
		if (dwWait == WAIT_OBJECT_0)
		{
			m_threadHandle = 0;
			m_threadId = 0;
			return this->Stop();
		}
	}
	return FALSE;
}
