

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 11:14:07 2038
 */
/* Compiler settings for IDiagProfiler.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.01.0628 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __IDiagProfiler_h_h__
#define __IDiagProfiler_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __IDiagProfiler_FWD_DEFINED__
#define __IDiagProfiler_FWD_DEFINED__
typedef interface IDiagProfiler IDiagProfiler;

#endif 	/* __IDiagProfiler_FWD_DEFINED__ */


#ifndef __DiagProfiler_FWD_DEFINED__
#define __DiagProfiler_FWD_DEFINED__

#ifdef __cplusplus
typedef class DiagProfiler DiagProfiler;
#else
typedef struct DiagProfiler DiagProfiler;
#endif /* __cplusplus */

#endif 	/* __DiagProfiler_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IDiagProfiler_INTERFACE_DEFINED__
#define __IDiagProfiler_INTERFACE_DEFINED__

/* interface IDiagProfiler */
/* [helpstring][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_IDiagProfiler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("53BBE57B-389D-428C-AB6F-79D7E7526EE0")
    IDiagProfiler : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetSamplingRate( 
            /* [in] */ int milliSeconds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSamplingRate( 
            /* [out] */ int *samplingRate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNoOfSamples( 
            /* [in] */ int noSamples) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNoOfSamples( 
            /* [out] */ int *noSamples) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StartSampling( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StopSampling( void) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IDiagProfilerVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiagProfiler * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiagProfiler * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiagProfiler * This);
        
        DECLSPEC_XFGVIRT(IDiagProfiler, SetSamplingRate)
        HRESULT ( STDMETHODCALLTYPE *SetSamplingRate )( 
            IDiagProfiler * This,
            /* [in] */ int milliSeconds);
        
        DECLSPEC_XFGVIRT(IDiagProfiler, GetSamplingRate)
        HRESULT ( STDMETHODCALLTYPE *GetSamplingRate )( 
            IDiagProfiler * This,
            /* [out] */ int *samplingRate);
        
        DECLSPEC_XFGVIRT(IDiagProfiler, SetNoOfSamples)
        HRESULT ( STDMETHODCALLTYPE *SetNoOfSamples )( 
            IDiagProfiler * This,
            /* [in] */ int noSamples);
        
        DECLSPEC_XFGVIRT(IDiagProfiler, GetNoOfSamples)
        HRESULT ( STDMETHODCALLTYPE *GetNoOfSamples )( 
            IDiagProfiler * This,
            /* [out] */ int *noSamples);
        
        DECLSPEC_XFGVIRT(IDiagProfiler, StartSampling)
        HRESULT ( STDMETHODCALLTYPE *StartSampling )( 
            IDiagProfiler * This);
        
        DECLSPEC_XFGVIRT(IDiagProfiler, StopSampling)
        HRESULT ( STDMETHODCALLTYPE *StopSampling )( 
            IDiagProfiler * This);
        
        END_INTERFACE
    } IDiagProfilerVtbl;

    interface IDiagProfiler
    {
        CONST_VTBL struct IDiagProfilerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiagProfiler_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDiagProfiler_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDiagProfiler_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDiagProfiler_SetSamplingRate(This,milliSeconds)	\
    ( (This)->lpVtbl -> SetSamplingRate(This,milliSeconds) ) 

#define IDiagProfiler_GetSamplingRate(This,samplingRate)	\
    ( (This)->lpVtbl -> GetSamplingRate(This,samplingRate) ) 

#define IDiagProfiler_SetNoOfSamples(This,noSamples)	\
    ( (This)->lpVtbl -> SetNoOfSamples(This,noSamples) ) 

#define IDiagProfiler_GetNoOfSamples(This,noSamples)	\
    ( (This)->lpVtbl -> GetNoOfSamples(This,noSamples) ) 

#define IDiagProfiler_StartSampling(This)	\
    ( (This)->lpVtbl -> StartSampling(This) ) 

#define IDiagProfiler_StopSampling(This)	\
    ( (This)->lpVtbl -> StopSampling(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDiagProfiler_INTERFACE_DEFINED__ */



#ifndef __DiagProfilerLib_LIBRARY_DEFINED__
#define __DiagProfilerLib_LIBRARY_DEFINED__

/* library DiagProfilerLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_DiagProfilerLib;

EXTERN_C const CLSID CLSID_DiagProfiler;

#ifdef __cplusplus

class DECLSPEC_UUID("C6DBEE4B-017D-43AC-8689-3B107A6104EF")
DiagProfiler;
#endif
#endif /* __DiagProfilerLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


