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

// The server uses asynchronous/non-blocking IO for communication
// There are very few situations where you need asynchronous IO.
// In a small piece of software like this, it is not really needed.
// But I wanted to know more about how to implement AsyncIO.
// So, enjoy :)
#include <Windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <tchar.h>
#include <strsafe.h>

#include "PipeServerAsync.h"
#include "FileLogger.h"
#include "DiagGlobals.h"

using namespace std;


VOID WINAPI CompletedReadRoutine(DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap)
{
	if ((dwErr == 0) && (cbBytesRead != 0)) 
	{ 
		LPPIPEINST pipeInst = (LPPIPEINST)lpOverLap;
		pipeInst->BytesToRead = cbBytesRead;
        g_debugLogger << "CompletedReadRoutine: BytesRead=" << cbBytesRead << endl;
	}
	else
	{
        g_debugLogger << "CompletedReadRoutine : dwErr=" << dwErr << endl;
	}
}


VOID WINAPI CompletedWriteRoutine(DWORD dwErr, DWORD cbWritten, LPOVERLAPPED lpOverLap) 
{
	LPPIPEINST pipeInst = (LPPIPEINST)lpOverLap;
	if ((dwErr == 0) && (cbWritten == pipeInst->BytesToWrite))
	{
        g_debugLogger << "CompletedWriteRoutine: BytesWritten=" << cbWritten << endl;

		LPPIPEINST pipeInst = (LPPIPEINST)lpOverLap;
		pipeInst->BytesToWrite = 0;
	}
	else
	{
        g_debugLogger << "CompletedWriteRoutine : dwErr=" << dwErr << endl;
	}
}


PipeServerAsync::PipeServerAsync()
{
	ZeroMemory(&m_pipeInstance, sizeof(m_pipeInstance));
    m_pipeInstance.PipeHandle = INVALID_HANDLE_VALUE;
    m_pipeInstance.StopEvent = INVALID_HANDLE_VALUE;
    m_pipeInstance.IoOverlapped.hEvent = INVALID_HANDLE_VALUE;
	m_pipeInstance.TimeoutMs = 5000;
}

PipeServerAsync::~PipeServerAsync()
{
	this->Stop();
}

BOOL PipeServerAsync::IsConnected()
{
	return m_pipeInstance.Connected;
}

void PipeServerAsync::Start()
{
	if (m_pipeInstance.PipeHandle != INVALID_HANDLE_VALUE)
		throw "Pipe already initialized";

	DWORD pId = GetCurrentProcessId();
	std::ostringstream o;
	o << "\\\\.\\pipe\\PipeServer_";
	o << pId;
	std::string pipeName = o.str();

    g_debugLogger << "Pipe = " << pipeName.c_str() << endl;
 
	m_pipeInstance.PipeHandle = CreateNamedPipeA( 
		pipeName.c_str(),		  // pipe name 
		PIPE_ACCESS_DUPLEX |      // read/write access 
		FILE_FLAG_OVERLAPPED,     // overlapped mode 
		PIPE_TYPE_MESSAGE |       // message-type pipe 
		PIPE_READMODE_MESSAGE |   // message read mode 
		PIPE_WAIT,                // blocking mode 
		1, // 1 instance 
		PIPE_MESSAGE_SIZE*sizeof(CHAR),    // output buffer size 
		PIPE_MESSAGE_SIZE*sizeof(CHAR),    // input buffer size 
		m_pipeInstance.TimeoutMs,             // client time-out 
		NULL);                    // default security attributes
	if (m_pipeInstance.PipeHandle == INVALID_HANDLE_VALUE) 
	{
		throw "CreateNamedPipe failed";
	}
	m_pipeInstance.IoOverlapped.hEvent = CreateEvent(
													NULL,    // default security attribute
													FALSE,    // manual reset event 
													FALSE,    // initial state = signaled 
													NULL);   // unnamed event object
	m_pipeInstance.StopEvent = CreateEvent(
													NULL,    // default security attribute
													FALSE,    // manual reset event 
													FALSE,    // initial state = signaled 
													NULL);   // unnamed event object
}
		
BOOL PipeServerAsync::ConnectToClientBegin() 
{ 
	if (m_pipeInstance.PipeHandle == INVALID_HANDLE_VALUE)
		throw "Not Started";

	BOOL connected = ConnectNamedPipe(m_pipeInstance.PipeHandle, &m_pipeInstance.IoOverlapped); 
	if (connected) 
	{
		throw "Overlapped ConnectNamedPipe should return zero";
	}

	BOOL pendingIO = FALSE;
	switch (GetLastError()) 
	{ 
		case ERROR_IO_PENDING: 
			pendingIO = TRUE; 
			break; 
		//case ERROR_PIPE_CONNECTED: 
		//	if (SetEvent(m_pipeInstance.IoOverlapped.hEvent)) 
		//		break;
		//	throw "ConnectNamedPipe::SetEvent failed";
		default:
			throw "ConnectNamedPipe";
	}
	return pendingIO;
}
	
BOOL PipeServerAsync::ConnectToClientEnd(DWORD timeout)
{
	DWORD dwWait = WaitForSingleObjectEx(
		m_pipeInstance.IoOverlapped.hEvent,  // event object to wait for 
		timeout,         // waits indefinitely 
		TRUE);           // alertable wait enabled

	switch (dwWait)
	{
	case WAIT_OBJECT_0:
		{
			BOOL success = GetOverlappedResult(m_pipeInstance.PipeHandle, &m_pipeInstance.IoOverlapped, &m_pipeInstance.BytesToRead, FALSE);
            g_debugLogger << "ConnectToClientEnd : GetOverlappedResult=" << success << endl;
 			m_pipeInstance.Connected = TRUE;
			return success;// && resetSuccess;
		}
	case WAIT_TIMEOUT:
        g_debugLogger.WriteLine("ConnectToClientEnd - WAIT_TIMEOUT");
		return FALSE;
	case WAIT_ABANDONED:
		g_debugLogger.WriteLine("ConnectToClientEnd - WAIT_ABANDONED");
		return FALSE;
	case WAIT_IO_COMPLETION:
		g_debugLogger.WriteLine("ConnectToClientEnd - WAIT_IO_COMPLETION");
		return FALSE;
	default:
		return FALSE;
	}
}

BOOL PipeServerAsync::ReleaseWaiters()
{
	if (m_pipeInstance.PipeHandle == INVALID_HANDLE_VALUE) 
		return FALSE;
	if (!m_pipeInstance.Connected)
		return FALSE;
	return SetEvent(m_pipeInstance.StopEvent);
}

BOOL PipeServerAsync::WaitForIO(DWORD timeout)
{
	if (m_pipeInstance.PipeHandle == INVALID_HANDLE_VALUE) 
		return FALSE;
	if (!m_pipeInstance.Connected)
		return FALSE;

	HANDLE eventArray[] = { m_pipeInstance.StopEvent, m_pipeInstance.IoOverlapped.hEvent };

	DWORD dwWait = WaitForMultipleObjectsEx(sizeof(eventArray)/sizeof(HANDLE), eventArray, FALSE, timeout, TRUE);
	//DWORD dwWait = WaitForSingleObjectEx(
	//	m_pipeInstance.IoOverlapped.hEvent,  // event object to wait for 
	//	timeout,         // waits indefinitely 
	//	TRUE);           // alertable wait enabled
	switch (dwWait)
	{
	case WAIT_IO_COMPLETION:
		// reset by CompletedReadRoutine
		// reset by CompletedWriteRoutine
		return TRUE;
	case WAIT_OBJECT_0:
		// Not valid with CompletionRoutines
		// Must be StopEvent
		ResetEvent(m_pipeInstance.StopEvent);
		return FALSE;
	case WAIT_TIMEOUT:
		return FALSE;
	default:
		g_debugLogger << "dwWait = " << dwWait << endl;
		return FALSE;
	}
}

BOOL PipeServerAsync::HasPendingIO()
{
	return WaitForIO(0);
}

BOOL PipeServerAsync::Stop()
{
	BOOL disconnectResult = TRUE;
	BOOL pipeCloseResult = TRUE;
	BOOL eventCloseResult = TRUE;
	BOOL stopEventCloseResult = TRUE;

	if (m_pipeInstance.PipeHandle != INVALID_HANDLE_VALUE) 
	{
		if (m_pipeInstance.Connected)
		{
			disconnectResult = DisconnectNamedPipe(m_pipeInstance.PipeHandle);
		}
		pipeCloseResult = CloseHandle(m_pipeInstance.PipeHandle);
		m_pipeInstance.PipeHandle = INVALID_HANDLE_VALUE;
	}
	if (m_pipeInstance.IoOverlapped.hEvent != INVALID_HANDLE_VALUE)
	{
		eventCloseResult = CloseHandle(m_pipeInstance.IoOverlapped.hEvent);
		m_pipeInstance.IoOverlapped.hEvent = INVALID_HANDLE_VALUE;
	}
	if (m_pipeInstance.StopEvent != INVALID_HANDLE_VALUE)
	{
		stopEventCloseResult = CloseHandle(m_pipeInstance.StopEvent);
		m_pipeInstance.StopEvent = INVALID_HANDLE_VALUE;
	}
	ZeroMemory(&m_pipeInstance, sizeof(PIPEINST));
	
	BOOL result = disconnectResult && pipeCloseResult &&eventCloseResult && stopEventCloseResult;
	return result;
}

BOOL PipeServerAsync::ReceiveBegin()
{
	BOOL readResult = ReadFileEx( 
		m_pipeInstance.PipeHandle, 
		m_pipeInstance.InputBuffer, 
		PIPE_MESSAGE_SIZE, 
		(LPOVERLAPPED)&m_pipeInstance, 
		(LPOVERLAPPED_COMPLETION_ROUTINE) CompletedReadRoutine);
	return readResult;
}

BOOL PipeServerAsync::ReceiveEnd(std::string& receivedText)
{
	BOOL success = GetOverlappedResult(m_pipeInstance.PipeHandle, &m_pipeInstance.IoOverlapped, &m_pipeInstance.BytesToRead, FALSE);
	g_debugLogger << "Receive : GetOverlappedResult=" << success <<endl;
	if (success)
	{
		g_debugLogger <<"BytesToRead=" << m_pipeInstance.BytesToRead << endl;
		m_pipeInstance.InputBuffer[m_pipeInstance.BytesToRead] = 0;
		receivedText = std::string(m_pipeInstance.InputBuffer, m_pipeInstance.BytesToRead);
		m_pipeInstance.BytesToRead = 0;
	}
	return success;
}

BOOL PipeServerAsync::ReceiveEnd(std::wstring& receivedText)
{
	BOOL success = GetOverlappedResult(m_pipeInstance.PipeHandle, &m_pipeInstance.IoOverlapped, &m_pipeInstance.BytesToRead, FALSE);
	g_debugLogger << "Receive : GetOverlappedResult=" << success <<endl;
	if (success)
	{
		g_debugLogger <<"BytesToRead=" << m_pipeInstance.BytesToRead << endl;
		DWORD charsToRead = m_pipeInstance.BytesToRead / sizeof(wchar_t);
		wchar_t* inputBuffer = reinterpret_cast<LPWSTR>(m_pipeInstance.InputBuffer);
		inputBuffer[charsToRead] = 0;

		receivedText = std::wstring(inputBuffer, charsToRead);
		m_pipeInstance.BytesToRead = 0;
	}
	return success;
}


BOOL PipeServerAsync::SendBegin(std::string& message)
{
	if (HasPendingIO())
		return FALSE;
	HRESULT hr = StringCchCopyA(m_pipeInstance.OutputBuffer, PIPE_MESSAGE_SIZE, message.c_str());
	if (hr != S_OK)
		return FALSE;
	m_pipeInstance.BytesToWrite = lstrlenA(m_pipeInstance.OutputBuffer);
	BOOL writeResult = WriteFileEx( 
		m_pipeInstance.PipeHandle, 
		m_pipeInstance.OutputBuffer, 
		m_pipeInstance.BytesToWrite, 
		(LPOVERLAPPED)&m_pipeInstance, 
		(LPOVERLAPPED_COMPLETION_ROUTINE) CompletedWriteRoutine);
	return writeResult;
}

BOOL PipeServerAsync::SendBegin(std::wstring& message)
{
	if (HasPendingIO())
		return FALSE;
	wchar_t* outputBuffer = reinterpret_cast<LPWSTR>(m_pipeInstance.OutputBuffer);

	HRESULT hr = StringCchCopyW(outputBuffer, PIPE_MESSAGE_SIZE, message.c_str());
	if (hr != S_OK)
		return FALSE;
	m_pipeInstance.BytesToWrite = lstrlenW(outputBuffer)*sizeof(wchar_t);
	BOOL writeResult = WriteFileEx( 
		m_pipeInstance.PipeHandle, 
		m_pipeInstance.OutputBuffer, 
		m_pipeInstance.BytesToWrite, 
		(LPOVERLAPPED)&m_pipeInstance, 
		(LPOVERLAPPED_COMPLETION_ROUTINE) CompletedWriteRoutine);
	return writeResult;
}