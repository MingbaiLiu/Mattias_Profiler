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

#include <Windows.h>

class DiagProfilerFactory : public IClassFactory
{
public:
	DiagProfilerFactory();
	~DiagProfilerFactory();

    //interface IUnknown methods 
    HRESULT __stdcall QueryInterface(const IID& iid, void **ppObj);
    ULONG   __stdcall AddRef();
    ULONG   __stdcall Release();

    //interface IClassFactory methods 
    HRESULT __stdcall CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    HRESULT __stdcall LockServer(BOOL bLock) ; 

private:
    long m_nRefCount;
};
