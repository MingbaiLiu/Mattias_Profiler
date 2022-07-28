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

#include "DiagProfilerFactory.h"
#include "DiagProfiler.h"
#include "DiagInit.h"
#include "DiagGlobals.h"

using namespace std;

DiagInit g_diagInit;

DiagProfilerFactory::DiagProfilerFactory()
{
    m_nRefCount=0;
	InterlockedIncrement(&g_comObjectsInUse);
}

DiagProfilerFactory::~DiagProfilerFactory()
{
    g_debugLogger.WriteLine("DiagProfilerFactory::~DiagProfilerFactory");
    InterlockedDecrement(&g_comObjectsInUse);
}

HRESULT __stdcall DiagProfilerFactory::QueryInterface(
                                const IID& iid, 
                                void** ppv)
{   
    g_debugLogger.WriteLine("DiagProfilerFactory::QueryInterface");

	if ((iid == IID_IUnknown) || (iid == IID_IClassFactory))
	{
		*ppv = static_cast<IClassFactory*>(this) ; 
	}
	else
	{
		*ppv = NULL ;
		return E_NOINTERFACE ;
	}
	reinterpret_cast<IUnknown*>(*ppv)->AddRef() ;
	return S_OK ;
}

HRESULT __stdcall DiagProfilerFactory::CreateInstance(IUnknown* pUnknownOuter,
                                           const IID& iid,
                                           void** ppv) 
    {
    g_debugLogger.WriteLine("DiagProfilerFactory::CreateInstance");
    if (!g_diagInit.m_stackLogger.IsOpen())
    {
        // DIAG_PRF_STACKTRACE must be defined
        // If not the stack will not be saved
        // It should have been opened in g_giagInit

        // Not true anymore
        // Added a server command to be able to set the stack log file
        // return E_INVALIDARG;
    }

    if (pUnknownOuter != NULL)
    {
	    return CLASS_E_NOAGGREGATION ;
    }

    IDiagProfiler* pObject = new DiagProfiler();

    if (pObject == NULL)
    {
	    return E_OUTOFMEMORY ;
	}

    return pObject->QueryInterface(iid, ppv) ;
}

 ULONG __stdcall DiagProfilerFactory::AddRef()
{
	return InterlockedIncrement(&m_nRefCount) ;
}

 ULONG __stdcall DiagProfilerFactory::Release()
{
	long nRefCount=0;
	nRefCount=InterlockedDecrement(&m_nRefCount) ;
	if (nRefCount == 0) delete this;
	return nRefCount;
}

HRESULT __stdcall DiagProfilerFactory::LockServer(BOOL bLock) 
{
    return E_NOTIMPL;
}