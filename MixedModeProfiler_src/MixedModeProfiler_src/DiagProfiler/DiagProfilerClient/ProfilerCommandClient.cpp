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

#include "ProfilerCommandClient.h"
#include <iostream>
#include <string>
#include <sstream>
#include <tchar.h>
#include <strsafe.h>


#define PIPE_MESSAGE_SIZE 500
#define PIPE_BUFFER_MAX_SIZE ((PIPE_MESSAGE_SIZE + 1))

using namespace std;

static std::string ConvertStlString(std::wstring& text)
{    
    const std::string narrow(text.begin(), text.end());
    return narrow;
}

ProfilerCommandClient::ProfilerCommandClient()
{
	m_pipeToProfiler = nullptr;
}

ProfilerCommandClient::~ProfilerCommandClient()
{
	if (m_pipeToProfiler)
	{
		CloseHandle(m_pipeToProfiler);
		m_pipeToProfiler = nullptr;
	}
}
	
HRESULT ProfilerCommandClient::SendCommand(const char* command, std::string& commandResult)
{
	if (m_pipeToProfiler == nullptr)
		throw "Not attached with pipes";
	DWORD bytesToWrite = strlen(command); // Don´t send null character

	DWORD bytesWritten = 0;
	BOOL result = WriteFile(m_pipeToProfiler, command, bytesToWrite, &bytesWritten, NULL);
	if (result)
	{
		char buffer[PIPE_BUFFER_MAX_SIZE];
		DWORD numBytesRead = 0;
		result = ReadFile(
				m_pipeToProfiler,
				buffer, // the data from the pipe will be put here
				PIPE_BUFFER_MAX_SIZE, // number of bytes allocated
				&numBytesRead, // this will store number of bytes actually read
				NULL // not using overlapped IO
				);
		if (result)
		{
			buffer[numBytesRead] = '\0'; // null terminate the string
			printf("Number of bytes read: %i\n", numBytesRead);
			printf("Message = %s\n", buffer);
			commandResult = std::string(buffer);
		}
		else
		{
			printf("Failed to read data from the pipe.\n");
		}
	}

	return result ? S_OK : E_FAIL;

}

HRESULT ProfilerCommandClient::SendCommand(const char* command)
{
	std::string dummy;
	return this->SendCommand(command, dummy);
}

BOOL GetOutputPars(std::string& message, std::string& output)
{
	std::string::size_type dataIndex = message.find_last_of(']');
	if (dataIndex == std::string::npos)
	{
		return FALSE;
	}

	dataIndex++;
	int copyLen = message.size() - dataIndex;
	output = std::string(message, dataIndex, copyLen);
	return TRUE;
}

HRESULT ProfilerCommandClient::GetSampleRate(int& rate)
{
	char command[] = "getsamplerate";
	std::string outputPars;
	HRESULT hr = SendCommand(command, outputPars);
	if (hr == S_OK)
	{
		std::string result;
		if (GetOutputPars(outputPars, result))
		{
			rate = atoi(result.c_str());
			return S_OK;
		}
		return E_FAIL;
	}
	return hr;
}

HRESULT ProfilerCommandClient::GetSampleCount(int& noSamples)
{
	char command[] = "getsamplecount";
	std::string outputPars;
	HRESULT hr = SendCommand(command, outputPars);
	if (hr == S_OK)
	{
		std::string result;
		if (GetOutputPars(outputPars, result))
		{
			noSamples = atoi(result.c_str());
			return S_OK;
		}
		return E_FAIL;
	}
	return hr;
}

HRESULT ProfilerCommandClient::SetMaxSamples(int noSamples)
{
	std::ostringstream o;
	o << "setmaxsamples=";
	o << noSamples;
	std::string command = o.str();
	return SendCommand(command.c_str());
}

HRESULT ProfilerCommandClient::SetStackLoggerFile(std::string& stackOutputPath)
{
	std::ostringstream o;
	o << "setstackloggerfile=";
	o << stackOutputPath.c_str();
	std::string command = o.str();
	return SendCommand(command.c_str());
}

HRESULT ProfilerCommandClient::SetSampleRate(int rateMs)
{
	std::ostringstream o;
	o << "setsamplerate=";
	o << rateMs;
	std::string command = o.str();
	return SendCommand(command.c_str());
}


HRESULT ProfilerCommandClient::StartSampling()
{
	char command[] = "start";
	return SendCommand(command);
}

HRESULT ProfilerCommandClient::StopSampling()
{
	char command[] = "stop";
	return SendCommand(command);
}

HRESULT ProfilerCommandClient::ConnectToProfiler(DWORD pId, DWORD timeout)
{
	const int sleeptime = 250;
	int retries = timeout / sleeptime;
	HRESULT hr = S_OK;
	do
	{
		hr = ConnectToProfiler(pId);
		if (hr == S_OK)
			break; // Connected
		Sleep(sleeptime);
		retries--;
	} while (retries != 0);
	if (retries == 0)
	{
		printf("Failed to connect to target\n");
        return E_FAIL;
	}
	else
	{
		printf("Connected\n", retries);
        return S_OK;
	}
}

HRESULT ProfilerCommandClient::ConnectToProfiler(DWORD pId)
{
	std::wostringstream o;
	o << "\\\\.\\pipe\\PipeServer_";
	o << pId;
	std::wstring pipeName = o.str();
	
	cout << "Pipe = "<< ConvertStlString(pipeName).c_str() << endl;

	HANDLE pipe = CreateFile(
		pipeName.c_str(),
		GENERIC_READ |  // read and write access 
			GENERIC_WRITE, 
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

	if (pipe == INVALID_HANDLE_VALUE)
	{
		//throw "Failed to connect to pipe.";
		return E_FAIL;
	}
	DWORD pipeMode = PIPE_READMODE_MESSAGE;
	BOOL pipeState = SetNamedPipeHandleState(pipe, &pipeMode, 0, 0);
	if (!pipeState)
	{
		CloseHandle(pipe);
		//throw "SetNamedPipeHandleState failed";
		return E_FAIL;
	}
	m_pipeToProfiler = pipe;
	return S_OK;
}
