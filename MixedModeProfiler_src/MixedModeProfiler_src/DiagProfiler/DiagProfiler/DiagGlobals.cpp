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

#include <stdlib.h>
#include <stdio.h>
#include "DiagGlobals.h"

const int MAX_BUFFER_SIZE = 500;

std::string GetEnvVar(std::string const& key)
{
    char buffer[MAX_BUFFER_SIZE];
    size_t returnSize = 0;
    //returnSize = len + 1(null)
    getenv_s<MAX_BUFFER_SIZE>(&returnSize, buffer, key.c_str());
    //char* buffer = getenv( key.c_str() );
    return returnSize == 0 ? std::string("") : std::string(buffer, returnSize - 1);
}

std::string ConvertStlString(std::wstring& text)
{    
    const std::string narrow(text.begin(), text.end());
    return narrow;
}

std::wstring ConvertStlString(std::string& text)
{
    const std::wstring wide(text.begin(), text.end());
    return wide;
}

