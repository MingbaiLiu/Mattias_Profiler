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

#include "FileLogger.h"

void FileLogger::Open(const char* fileName)
{
    if (m_file.is_open())
	{
		throw "FileLogger file is already open";
	}
	if (fileName == nullptr)
		throw "FileLogger(fileName=nullptr)";

    m_file.open(fileName, std::ios_base::out | std::ios_base::trunc);
    if (m_file.fail())
    {
        throw "FileLogger.Open - failed";
    }

	m_fileName = std::string(fileName);
}

void FileLogger::Close()
{
    if (m_file.is_open())
    {
        m_file.flush();
        m_file.close();
    }
}

void FileLogger::WriteLine(const char* message)
{
    *this << message << std::endl;
}

void FileLogger::SetPrefix(const char* linePrefix)
{
    m_streamBuffer.SetPrefix(linePrefix);
}
