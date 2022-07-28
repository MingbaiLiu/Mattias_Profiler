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

#include <Windows.h>
#include <string>

#pragma once

#define PIPE_MESSAGE_SIZE 500
#define PIPE_BUFFER_MAX_SIZE ((PIPE_MESSAGE_SIZE + 1))

typedef struct 
{
	OVERLAPPED IoOverlapped;
	HANDLE StopEvent;
	HANDLE PipeHandle;
	DWORD BytesToWrite;
	DWORD BytesToRead;
	DWORD TimeoutMs;
	CHAR InputBuffer[PIPE_BUFFER_MAX_SIZE];
	CHAR OutputBuffer[PIPE_BUFFER_MAX_SIZE];
	BOOL Connected;
} PIPEINST, *LPPIPEINST; 


class PipeServerAsync
{
	PIPEINST m_pipeInstance;

public:
	PipeServerAsync();
	~PipeServerAsync();

	void Start();
	BOOL ConnectToClientBegin();
	BOOL ConnectToClientEnd(DWORD timeout);
	BOOL WaitForIO(DWORD timeout);
	BOOL ReleaseWaiters();
	BOOL HasPendingIO();
	BOOL Stop();
	BOOL ReceiveBegin();
	BOOL ReceiveEnd(std::string& receivedText);
	BOOL SendBegin(std::string& message);
	BOOL ReceiveEnd(std::wstring& receivedText);
	BOOL SendBegin(std::wstring& message);
	BOOL IsConnected();
};
