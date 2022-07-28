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

#include "FileLogger.h"
#include "LockedStream.h"
#include "DiagInit.h"
#include <string>


std::string GetEnvVar(std::string const& key);
std::wstring ConvertStlString(std::string& text);
std::string ConvertStlString(std::wstring& text);

extern DiagInit g_diagInit;
#define g_debugLogger (LockedStream(g_diagInit.m_debugLogger, g_diagInit.m_cs_debug))
#define g_stackLogger (LockedStream(g_diagInit.m_stackLogger, g_diagInit.m_cs_stack))
#define g_debugLoggerVerbose (LockedStream(g_diagInit.m_debugLoggerVerbose, g_diagInit.m_cs_debug))
#define g_nativeStackwalker (*g_diagInit.m_nativeStackwalker)
