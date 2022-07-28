

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Mon Jan 23 20:20:58 2012
 */
/* Compiler settings for IDiagProfiler.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __IDiagProfiler_h__
#define __IDiagProfiler_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
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
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IDiagProfiler_INTERFACE_DEFINED__
#define __IDiagProfiler_INTERFACE_DEFINED__

/* interface IDiagProfiler */
/* [unique][helpstring][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_IDiagProfiler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("53BBE57B-389D-428C-AB6F-79D7E7526EE0")
    IDiagProfiler : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetFirstNumber( 
            long nX1) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSecondNumber( 
            long nX2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DoTheAddition( 
            /* [retval][out] */ long *pBuffer) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDiagProfilerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiagProfiler * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiagProfiler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiagProfiler * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetFirstNumber )( 
            IDiagProfiler * This,
            long nX1);
        
        HRESULT ( STDMETHODCALLTYPE *SetSecondNumber )( 
            IDiagProfiler * This,
            long nX2);
        
        HRESULT ( STDMETHODCALLTYPE *DoTheAddition )( 
            IDiagProfiler * This,
            /* [retval][out] */ long *pBuffer);
        
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


#define IDiagProfiler_SetFirstNumber(This,nX1)	\
    ( (This)->lpVtbl -> SetFirstNumber(This,nX1) ) 

#define IDiagProfiler_SetSecondNumber(This,nX2)	\
    ( (This)->lpVtbl -> SetSecondNumber(This,nX2) ) 

#define IDiagProfiler_DoTheAddition(This,pBuffer)	\
    ( (This)->lpVtbl -> DoTheAddition(This,pBuffer) ) 

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


