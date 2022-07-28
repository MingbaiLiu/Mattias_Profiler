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

class PerformanceCounter
{
public:
    PerformanceCounter()
    {
        if (!QueryPerformanceFrequency( &m_ticksPerSecond ))
			throw "QueryPerformanceFrequency failed";
    }

    void Start()
    {
        m_start.QuadPart = 0;
        QueryPerformanceCounter( &m_start );
    }

    void Stop()
    {
        m_end.QuadPart = 0;
        QueryPerformanceCounter( &m_end );
    }

    long double GetDurationInSeconds()
    {
        return (m_end.QuadPart - m_start.QuadPart) / long double( m_ticksPerSecond.QuadPart );
    }

	long long GetDurationInTicks()
    {
        return (m_end.QuadPart - m_start.QuadPart);
    }

	long long GetTicksPerSecond()
	{
		return m_ticksPerSecond.QuadPart;
	}

private:
    LARGE_INTEGER m_start;
    LARGE_INTEGER m_end;
    LARGE_INTEGER m_ticksPerSecond;
};