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

#include "ProfilerCommand.h"

std::string ProfilerCommand::GetName()
{
	return m_name;
}

ProfilerCommand::ProfilerCommand(const char* name, PrfCmdFuncBoolStringVoid action)
{
	m_name = std::string(name);
	m_function = nullptr;
    //m_objectFunction = nullptr;
    //m_objectAction = nullptr;

   	m_action = action;
	if (m_action == nullptr)
		throw "ProfilerCommand(nullptr)";
}

//
//ProfilerCommand::ProfilerCommand(const char* name, PrfCmdMethBoolStringVoid action)
//{
//	m_name = std::string(name);
//	m_function = nullptr;
//    m_action = nullptr;
//    m_objectFunction = nullptr;
//
//	m_objectAction = action;
//	if (m_objectAction == nullptr)
//		throw "ProfilerCommand(nullptr)";
//}

ProfilerCommand::ProfilerCommand(const char* name, PrfCmdFuncBoolStringString function)
{
	m_name = std::string(name);
	m_action = nullptr;
    //m_objectFunction = nullptr;
    //m_objectAction = nullptr;

   	m_function = function;
	if (m_function == nullptr)
		throw "ProfilerCommand(nullptr)";
}

//ProfilerCommand::ProfilerCommand(const char* name, PrfCmdMethBoolStringString function)
//{
//	m_name = std::string(name);
//	m_action = nullptr;
//    m_function = nullptr;
//    m_objectAction = nullptr;
//
//    m_objectFunction = function;
//
//	if (m_objectFunction == nullptr)
//		throw "ProfilerCommand(nullptr)";
//}

bool ProfilerCommand::Execute(std::string& outputResult)
{
	if (m_action != nullptr)
	{
		return m_action(outputResult);
	}
    //if (m_objectAction != nullptr)
    //{
    //    return m_objectAction(outputResult);
    //}
	throw "ProfilerCommand::Execute nullptr";
}

bool ProfilerCommand::Execute(std::string& outputResult, std::string& inputPars)
{
	if (m_function != nullptr)
	{
		return m_function(outputResult, inputPars);
	}
	//if (m_objectFunction != nullptr)
	//{
	//	return m_objectFunction(outputResult, inputPars);
	//}
    throw "ProfilerCommand::Execute nullptr";
}