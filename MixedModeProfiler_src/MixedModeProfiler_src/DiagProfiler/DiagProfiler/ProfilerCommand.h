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
#include <string>

typedef bool (*PrfCmdFuncBoolStringVoid)(std::string&);
typedef bool (*PrfCmdFuncBoolStringString)(std::string&, std::string&);

class ProfilerCommand
{
private:
	PrfCmdFuncBoolStringVoid m_action;
	PrfCmdFuncBoolStringString m_function;
    std::string m_name;
public:
	std::string GetName();
private:
	ProfilerCommand() {}
public:
	~ProfilerCommand() {}
	ProfilerCommand(const char* name, PrfCmdFuncBoolStringVoid action);
	ProfilerCommand(const char* name, PrfCmdFuncBoolStringString function);
	virtual bool Execute(std::string& outputResult);
	virtual bool Execute(std::string& outputResult, std::string& inputPars);
};