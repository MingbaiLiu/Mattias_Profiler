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

#include "DiagProfilerServer.h"
#include "IDiagProfiler_h.h"
#include "FileLogger.h"
#include "LockedStream.h"
#include "DiagGlobals.h"
#include <sstream>

static IDiagProfiler* g_diagProfiler = nullptr;

bool CommandGetSampleRate(std::string& outputResult);
bool CommandGetSampleCount(std::string& outputResult);
bool CommandStartSampling(std::string& outputResult);
bool CommandStopSampling(std::string& outputResult);
bool CommandSetMaxSamples(std::string& outputResult, std::string& inputPars);
bool CommandSetStackLoggerFile(std::string& outputResult, std::string& inputPars);
bool CommandSetSampleRate(std::string& outputResult, std::string& inputPars);


DiagProfilerServer::DiagProfilerServer()
{
}

DiagProfilerServer::~DiagProfilerServer()
{
}

BOOL DiagProfilerServer::Start(IDiagProfiler* diagProfiler, DWORD timeout)
{
    g_diagProfiler = diagProfiler;
    BOOL connected = m_commandServer.Start(timeout);
    if (connected)
    {
        m_commandServer.RegisterCommand("start", CommandStartSampling);
		m_commandServer.RegisterCommand("stop", CommandStopSampling);
		m_commandServer.RegisterCommand("getsamplecount", CommandGetSampleCount);
		m_commandServer.RegisterCommand("getsamplerate", CommandGetSampleRate);
		m_commandServer.RegisterCommand("setstackloggerfile", CommandSetStackLoggerFile);
		m_commandServer.RegisterCommand("setmaxsamples", CommandSetMaxSamples);
        m_commandServer.RegisterCommand("setsamplerate", CommandSetSampleRate);
    }
    return connected;
}

bool CommandGetSampleRate(std::string& outputResult)
{
	g_debugLogger.WriteLine("CommandGetSampleRate");
	if (g_diagProfiler != nullptr)
	{
		int rate = 0;
		HRESULT hr = g_diagProfiler->GetSamplingRate(&rate);
        bool result = hr == S_OK;
		if (result)
		{
			std::ostringstream o;
			o << rate;
			outputResult = o.str();
            g_debugLogger << "Response: " << rate << std::endl;
		}
		return result;
	}
	g_debugLogger.WriteLine("CommandGetSampleRate : failed");
	return false;
}

bool CommandGetSampleCount(std::string& outputResult)
{
	g_debugLogger.WriteLine("CommandGetSampleCount");
	if (g_diagProfiler != nullptr)
	{
		int noSamples = 0;
		HRESULT hr = g_diagProfiler->GetNoOfSamples(&noSamples);
        bool result = hr == S_OK;
		if (result)
		{
			std::ostringstream o;
			o << noSamples;
			outputResult = o.str();
            g_debugLogger << "Response: " << outputResult.c_str() << std::endl;
		}
		return result;
	}
	g_debugLogger.WriteLine("CommandGetSampleCount : failed");
	return false;
}

bool CommandSetStackLoggerFile(std::string& outputResult, std::string& inputPars)
{
	g_debugLogger.WriteLine("CommandSetStackLoggerFile");
	if (g_diagProfiler != nullptr)
	{
        if (g_diagInit.m_debugLogger.IsOpen())
        {
           	g_debugLogger.WriteLine("CommandSetStackLoggerFile : failed - a file is already open");
            outputResult = std::string("StackLogger file is already open");
            return false;
        }
        else
        {
            g_debugLogger << "Trying to open file " << inputPars.c_str() << std::endl;
            g_stackLogger.Open(inputPars.c_str());
            if (g_diagInit.m_stackLogger.IsOpen())
            {
                g_debugLogger.WriteLine("Open stack log file successful");
                return true;
            }
            else
            {
                outputResult = std::string("StackLogger failed to open");
                return false;                
            }
        }
	}
	g_debugLogger.WriteLine("CommandSetStackLoggerFile : failed");
	return false;
}

bool CommandSetMaxSamples(std::string& outputResult, std::string& inputPars)
{
	g_debugLogger.WriteLine("CommandSetNoSamples");
	if (g_diagProfiler != nullptr)
	{
		int noSamples = atoi(inputPars.c_str());
		HRESULT hr = g_diagProfiler->SetNoOfSamples(noSamples);
        bool result = hr == S_OK;
        return result;
	}
	g_debugLogger.WriteLine("CommandSetNoSamples : failed");
	return false;
}

bool CommandSetSampleRate(std::string& outputResult, std::string& inputPars)
{
	g_debugLogger.WriteLine("CommandSetSampleRate");
	if (g_diagProfiler != nullptr)
	{
		int noSamples = atoi(inputPars.c_str());
        HRESULT hr = g_diagProfiler->SetSamplingRate(noSamples);
        bool result = hr == S_OK;
        return result;
	}
	g_debugLogger.WriteLine("CommandSetSampleRate : failed");
	return false;
}

bool CommandStartSampling(std::string& outputResult)
{
	g_debugLogger.WriteLine("CommandStartSampling");
    if (!g_diagInit.m_stackLogger.IsOpen())
    {
	    g_debugLogger.WriteLine("CommandStartSampling : failed");
    	g_debugLogger.WriteLine("Stack log file is not configured");
        return false;
    }
	if (g_diagProfiler != nullptr)
	{
		HRESULT hr = g_diagProfiler->StartSampling();
        bool result = hr == S_OK;
        if (result)
        {
            g_diagInit.m_debugLogger.Echo2OutputDebug(false);
        }
        else
        {
	        g_debugLogger.WriteLine("CommandStartSampling : failed");
        }
        return result;
	}
	g_debugLogger.WriteLine("CommandStartSampling : failed");
	return false;
}

bool CommandStopSampling(std::string& outputResult)
{
	g_debugLogger.WriteLine("CommandStopSampling");
	if (g_diagProfiler != nullptr)
	{
		HRESULT hr = g_diagProfiler->StopSampling();
        bool result = hr == S_OK;
        if (result)
        {
            g_diagInit.m_debugLogger.Echo2OutputDebug(true);
           	g_debugLogger.WriteLine("CommandStopSampling : Succeeded");
        }
        return result;
	}
	g_debugLogger.WriteLine("CommandStopSampling : failed");
	return false;
}
