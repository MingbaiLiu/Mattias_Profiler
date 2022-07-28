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

// CppCliApp.cpp : main project file.

#include "stdafx.h"

using namespace System;

void myreadline()
{
    System::Console::ReadLine();
}

#pragma unmanaged
int nativeA(int a)
{
    myreadline();
    return a * 1;
}

#pragma managed 

int managedAB(int a, int b) { return b * nativeA(a); }



int main(array<System::String ^> ^args)
{
    int pId = System::Diagnostics::Process::GetCurrentProcess()->Id;
    Console::Write("pId = ");
    Console::WriteLine(pId);
    int result = managedAB(1, 2);
    Console::Write("Result = ");
    Console::WriteLine(result);
    return 0;
}
