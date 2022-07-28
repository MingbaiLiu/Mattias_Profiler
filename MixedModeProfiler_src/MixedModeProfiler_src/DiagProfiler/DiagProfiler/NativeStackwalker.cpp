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

#include <windows.h>
#include <dbghelp.h>
#include <iostream>
#include <list>
#include <string>
#include <tlhelp32.h>

#include "NativeStackwalker.h"
#include "FileLogger.h"
#include "DiagGlobals.h"

using namespace std;

struct MyModule
{
	std::string ModuleName;
    DWORD64 ModuleBase;
    ULONG ModuleSize;
};

std::list<MyModule> g_loadedModules;

HANDLE g_currentProcess = 0;

NativeStackwalker::~NativeStackwalker(void)
{
	if (m_currentProcess != INVALID_HANDLE_VALUE)
	{
		SymCleanup(m_currentProcess);
		CloseHandle(m_currentProcess);
		m_currentProcess = INVALID_HANDLE_VALUE;
	}
	CurrentThread = INVALID_HANDLE_VALUE;
}

BOOL __stdcall MySymbolCallback
(
    __in PSYMBOL_INFO pSymInfo,
    __in ULONG SymbolSize,
    __in_opt PVOID UserContext
    )
{
    g_debugLogger << "SymbolName = " << pSymInfo->Name << endl;
    g_debugLogger << "Address    = 0x" << hex << pSymInfo->Address << endl;
    g_debugLogger << "ModBase    = 0x" << hex << pSymInfo->ModBase << endl;
	return TRUE;
}

BOOL __stdcall MyModuleCallback
(
    __in PCSTR ModuleName,
    __in DWORD64 BaseOfDll,
    __in_opt PVOID UserContext
)
{
    g_debugLogger << "ModuleName = " << ModuleName << endl;
	g_debugLogger << "BaseOfDll  = 0x" << hex << BaseOfDll << endl;
	if (strcmp(ModuleName, "C:\\Developer\\NativeLib.dll") == 0)
	{
		if (!SymEnumSymbols(g_currentProcess, BaseOfDll, "", MySymbolCallback, NULL))
			g_debugLogger.WriteLine("SymEnumSymbols failed");
		else
			g_debugLogger.WriteLine("SymEnumSymbols succeeded");
	}
	return TRUE;
};



BOOL __stdcall MyModuleCallback2
	(
    __in PCSTR ModuleName,
    __in DWORD64 ModuleBase,
    __in ULONG ModuleSize,
    __in_opt PVOID UserContext
	)
{
    g_debugLogger << "ModuleName = " << ModuleName << endl;
	g_debugLogger << "ModuleBase  = 0x" << hex << ModuleBase << endl;
	g_debugLogger << "ModuleSize  = 0x%" << hex << ModuleSize << endl;

	MyModule* module = new MyModule();
	module->ModuleBase = ModuleBase;
	module->ModuleName = std::string(ModuleName);
	module->ModuleSize = ModuleSize;

	g_loadedModules.push_back(*module);

	return TRUE;
}

bool NativeStackwalker::ForceLoad(DWORD64 addr)
{
	return false;
	int size = g_loadedModules.size();
    g_debugLogger << "Searching for loaded module(" << hex << size << "): 0x" << hex << addr << endl;
		
	for each(MyModule module in g_loadedModules)
	{
		DWORD64 moduleEnd = module.ModuleBase + module.ModuleSize;
		if ((module.ModuleBase <= addr) && (addr <= moduleEnd))
		{
            g_debugLogger << "Found range: 0x" << hex << module.ModuleBase << " - 0x" << hex << moduleEnd << endl;
			LoadPdb(std::string(), module.ModuleBase);
			return true;
		}		
	}
	return false;
}

bool GetFileSize( const std::wstring fileName, DWORD& FileSize )
{
	if( fileName.size() == 0 ) 
	{
		return false; 
	}
	HANDLE hFile = ::CreateFile(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL ); 
	if( hFile == INVALID_HANDLE_VALUE ) 
	{
		return false; 
	}
	FileSize = ::GetFileSize( hFile, NULL ); 
	if( FileSize == INVALID_FILE_SIZE ) 
	{
		// and continue ... 
	}
	if( !::CloseHandle( hFile ) ) 
	{
		// and continue ... 
	}
	return ( FileSize != INVALID_FILE_SIZE );
}

void NativeStackwalker::LoadPdb(std::string& fileName, DWORD64 baseAddr)
{
	std::wstring fileNameW(fileName.begin(),fileName.end());
	fileNameW.assign(fileName.begin(), fileName.end());

	DWORD fileSize = 0;
	if (GetFileSize(fileNameW.c_str(), fileSize))
	{
		//DWORD64 BaseAddr = 0x10000000;
		DWORD64 ModBase = SymLoadModule64 ( 
								m_currentProcess, // Process handle of the current process 
								NULL,                // Handle to the module's image file (not needed)
								fileName.c_str(),           // Path/name of the file 
								NULL,                // User-defined short name of the module (it can be NULL) 
								baseAddr,            // Base address of the module (cannot be NULL if .PDB file is used, otherwise it can be NULL) 
								fileSize             // Size of the file (cannot be NULL if .PDB file is used, otherwise it can be NULL) 
							);
		if( ModBase == 0 ) 
		{
            g_debugLogger << "Error: SymLoadModule64() failed. Error code: 0x" << hex << GetLastError() << endl;
		}
        g_debugLogger << "Load address: 0x" << hex << ModBase << endl;
	}
}

std::string NativeStackwalker::GetEnvSymbolPath()
{
    std::string envSymbolPath = std::string("DIAG_PRF_SYMBOLPATH");
    std::string symbolPath = GetEnvVar(envSymbolPath);
    return symbolPath;
}

std::string NativeStackwalker::GetSymbolPath()
{
	char alternativePath[] = "SRV*c:\\symbols*http://msdl.microsoft.com/download/symbols;D:\\junFiles\\IProfiler\\MixedModeProfiler_src\\MixedModeProfiler_src\\IProfiler\\IProfiler\\MixedModeProfiler_src\\MixedModeProfiler_src\\DiagProfiler\\Debug\\";
    if (m_symbolPath.size() == 0)
    {
        m_symbolPath = GetEnvSymbolPath();

        if (m_symbolPath.size() == 0)
        {
	        CHAR buffer[500];
	        DWORD bufferSize = sizeof(buffer)*sizeof(CHAR);
	        DWORD len = GetCurrentDirectoryA(bufferSize, buffer);
	        if ((len > bufferSize) || (len == 0))
		        throw "GetCurrentDirectory failed";

            g_debugLogger << "Default Symbol Path = " << " <Current Dir>" << endl;
            g_debugLogger << "CurrentDir = " << buffer << endl;
            g_debugLogger << "Consider setting the path to the pdb files manually" << endl;
            g_debugLogger << "SET DIAG_PRF_SYMBOLPATH=" << alternativePath << endl;

            m_symbolPath = std::string(buffer);
        }
        else
        {
            g_debugLogger << "DIAG_PRF_SYMBOLPATH=" << m_symbolPath.c_str() << endl;
        }
    }
    return m_symbolPath;
}

NativeStackwalker::NativeStackwalker(void) : CurrentThread(INVALID_HANDLE_VALUE)
{
	m_currentProcess = GetCurrentProcess();
	g_currentProcess = m_currentProcess;

	m_sym_is_initialized = SymInitialize(m_currentProcess, nullptr, TRUE) == TRUE;
	DWORD symOptions = SymGetOptions();
	
	symOptions |= SYMOPT_LOAD_LINES;
	symOptions |= SYMOPT_FAIL_CRITICAL_ERRORS;
	symOptions |= SYMOPT_LOAD_ANYTHING;
	//symOptions |= SYMOPT_NO_PROMPTS;
	symOptions |= SYMOPT_CASE_INSENSITIVE;
	symOptions |= SYMOPT_AUTO_PUBLICS;
	symOptions |= SYMOPT_OMAP_FIND_NEAREST;
	//symOptions |= SYMOPT_INCLUDE_32BIT_MODULES;
	symOptions |= SYMOPT_UNDNAME;
	//symOptions |= SYMOPT_IGNORE_CVREC;
	//symOptions |= SYMOPT_NO_PUBLICS;

	SymSetOptions(symOptions);

   	std::string symbolPath = GetSymbolPath();
    SetSymbolPath(symbolPath);
}

void NativeStackwalker::SetSymbolPath(std::string path)
{
    BOOL r1 = SymSetSearchPath(m_currentProcess, path.c_str());
	BOOL r2 = SymRefreshModuleList(m_currentProcess);
    g_debugLogger << "Set Symbol Path = " << path.c_str() << endl;
}

void NativeStackwalker::DisplayDebugData()
{
	static bool displayed = false;

	if (displayed == false)
	{
		displayed = true;
		g_loadedModules.clear();
		EnumerateLoadedModules64(m_currentProcess, MyModuleCallback2, NULL);
	}
}

HRESULT NativeStackwalker::ListProcessThreads(DWORD dwOwnerPID, std::list<DWORD>& threadList)
{
	threadList.clear();
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
	THREADENTRY32 te32;

	// Take a snapshot of all running threads
	hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 );
	if( hThreadSnap == INVALID_HANDLE_VALUE )
	{
		return (S_FALSE);
	}
	te32.dwSize = sizeof(THREADENTRY32 );

	if( !Thread32First( hThreadSnap, &te32 ) )
	{
        g_debugLogger.WriteLine("Thread32First failed");
		CloseHandle( hThreadSnap );
		return (S_FALSE);
	}

	g_debugLogger.WriteLine("--- Listing all Threads - BEGIN --");

	do
	{
		if( te32.th32OwnerProcessID == dwOwnerPID )
		{
			g_debugLogger << "ThreadId = 0x" << te32.th32ThreadID << endl;
            g_debugLogger << "   base priority = " << te32.tpBasePri << endl;
            g_debugLogger << "   delta priority = " << te32.tpDeltaPri << endl;
			threadList.push_back(te32.th32ThreadID);
		}
	} while( Thread32Next(hThreadSnap, &te32 ) );
	g_debugLogger.WriteLine("--- Listing all Threads - END --");

	CloseHandle( hThreadSnap );
	return( S_OK );
}

DWORD64 NativeStackwalker::GetNextEIP(DWORD64 EIP, DWORD64 EBP, DWORD64 ESP, int skip)
{
	std::list<DWORD64> stackFrames;
	if (!m_sym_is_initialized)
    {
		return 0;
    }	

	if (CurrentThread == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	
	CONTEXT newContext = {0};
	newContext.Eip = static_cast<DWORD>(EIP);
	newContext.Ebp = static_cast<DWORD>(EBP);
	newContext.Esp = static_cast<DWORD>(ESP);

	STACKFRAME64 stackFrame = { 0 };
	stackFrame.AddrPC.Offset         = EIP;
    stackFrame.AddrPC.Mode           = AddrModeFlat;
    stackFrame.AddrFrame.Offset      = EBP;
    stackFrame.AddrFrame.Mode        = AddrModeFlat;
    stackFrame.AddrStack.Offset      = ESP;
    stackFrame.AddrStack.Mode        = AddrModeFlat;
	
	int addedFrames = 0;
	   while(
    StackWalk64(
        IMAGE_FILE_MACHINE_I386,
        m_currentProcess,
        CurrentThread,
        &stackFrame,
        &newContext,
        NULL,
        SymFunctionTableAccess64,
        SymGetModuleBase64,
        NULL))
    {
	    if(skip > 0)
		{
			--skip;
		}
		else
		{
			return stackFrame.AddrPC.Offset;
		}
    }
    return 0;
}

std::list<DWORD64> NativeStackwalker::WalkFrames(const CONTEXT& context, int skip, int max)
{
	std::list<DWORD64> stackFrames;
	if (!m_sym_is_initialized)
    {
		return stackFrames;
    }	

	if (CurrentThread == INVALID_HANDLE_VALUE)
	{
		return stackFrames;
		//throw "WalkStack2 - CurrentThread == INVALID_HANDLE_VALUE";
	}
	CONTEXT newContext;
	memcpy(&newContext, &context, sizeof(CONTEXT));

	STACKFRAME64 stackFrame = { 0 };
	stackFrame.AddrPC.Offset         = context.Eip;
    stackFrame.AddrPC.Mode           = AddrModeFlat;
    stackFrame.AddrFrame.Offset      = context.Ebp;
    stackFrame.AddrFrame.Mode        = AddrModeFlat;
    stackFrame.AddrStack.Offset      = context.Esp;
    stackFrame.AddrStack.Mode        = AddrModeFlat;
	
	int addedFrames = 0;
	   while(
    StackWalk64(
        IMAGE_FILE_MACHINE_I386,
        m_currentProcess,
        CurrentThread,
        &stackFrame,
        &newContext,
        NULL,
        SymFunctionTableAccess64,
        SymGetModuleBase64,
        NULL))
    {
	    if(skip > 0)
		{
			--skip;
		}
		else
		{
			addedFrames++;
			stackFrames.push_back(stackFrame.AddrPC.Offset);
			if (addedFrames == max)
			{
				break;
			}
		}
    }
    return stackFrames;
}

std::list<DWORD64> NativeStackwalker::WalkStack2(const CONTEXT& context)
{
	std::list<DWORD64> stackFrames;
	if (!m_sym_is_initialized)
    {
		return stackFrames;
    }	

	if (CurrentThread == INVALID_HANDLE_VALUE)
	{
		return stackFrames;
	}
	CONTEXT newContext;
	memcpy(&newContext, &context, sizeof(CONTEXT));

	STACKFRAME64 stackFrame = { 0 };
	stackFrame.AddrPC.Offset         = context.Eip;
    stackFrame.AddrPC.Mode           = AddrModeFlat;
    stackFrame.AddrFrame.Offset      = context.Ebp;
    stackFrame.AddrFrame.Mode        = AddrModeFlat;
    stackFrame.AddrStack.Offset      = context.Esp;
    stackFrame.AddrStack.Mode        = AddrModeFlat;
	
	SetLastError(0);
	
	while(
    StackWalk64(
        IMAGE_FILE_MACHINE_I386,
        m_currentProcess,
        CurrentThread,
        &stackFrame,
        &newContext,
        NULL,
        SymFunctionTableAccess64,
        SymGetModuleBase64,
        NULL))
    {
        //g_debugLogger << "   Pushing 0x" << hex << stackFrame.AddrPC.Offset << endl;
		stackFrames.push_back(stackFrame.AddrPC.Offset);
    }
	DWORD last = GetLastError();
	if (last != 0)
    {
        g_debugLogger << "LastError 0x" << hex << last << endl;
    }

    return stackFrames;
}

std::list<DWORD64> NativeStackwalker::WalkStack(int skipFrame, HANDLE processHandle, HANDLE threadHandle)
{
	if (processHandle == NULL)
		processHandle = GetCurrentProcess();
	std::list<DWORD64> stackFrames;
    if (!m_sym_is_initialized)
    {
		return stackFrames;
    }		 
		 
    STACKFRAME64 stackFrame = { 0 };
    CONTEXT context         = { 0 };

    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);

    stackFrame.AddrPC.Offset         = context.Eip;
    stackFrame.AddrPC.Mode           = AddrModeFlat;
    stackFrame.AddrFrame.Offset      = context.Ebp;
    stackFrame.AddrFrame.Mode        = AddrModeFlat;
    stackFrame.AddrStack.Offset      = context.Esp;
    stackFrame.AddrStack.Mode        = AddrModeFlat;

    while(
    StackWalk64(
        IMAGE_FILE_MACHINE_I386,
        processHandle,
        threadHandle,
        &stackFrame,
        &context,
        NULL,
        SymFunctionTableAccess64,
        SymGetModuleBase64,
        NULL))
    {
    if(skipFrame > 0)
    {
        --skipFrame;
    }
    else
    {
		stackFrames.push_back(stackFrame.AddrPC.Offset);
    }
    }	
    return stackFrames;
}

const char* ConvertModuleType2String(SYM_TYPE symType)
{
	switch (symType)
        {
        case SymNone:
          return "-nosymbols-";
          break;
        case SymCoff:
          return "COFF";
          break;
        case SymCv:
          return  "CV";
          break;
        case SymPdb:
          return "PDB";
          break;
        case SymExport:
          return "-exported-";
          break;
        case SymDeferred:
          return "-deferred-";
          break;
        case SymSym:
          return "SYM";
          break;
        case SymDia:
          return "DIA";
          break;
        case 8: //SymVirtual:
          return "Virtual";
          break;
        default:
          //_snprintf( ty, sizeof ty, "symtype=%ld", (long) Module.SymType );
          return "NULL";
          break;
        }
}

HRESULT NativeStackwalker::AnnotateStack(DWORD64 addr64, std::string& result)
{
	static int frameCounter = 0;
    g_debugLoggerVerbose << "Annotate:  Addr64 = 0x" << hex << addr64 << endl;

	frameCounter++;
	if(!m_sym_is_initialized)
	{
		result = std::string("Please call SymInitialized before using this function");
		return S_FALSE;
	} 
    IMAGEHLP_LINE64 line = { 0 };
	const int NAME_BUFFER_LEN = 255;
	char buffer[NAME_BUFFER_LEN];
	char symbolbytes[sizeof(IMAGEHLP_SYMBOL64) + 256 * sizeof(char)];

	result.clear();

	IMAGEHLP_MODULE64 ModuleInfo = { 0 };
	PIMAGEHLP_SYMBOL64 SymbolInfo =  reinterpret_cast<PIMAGEHLP_SYMBOL64>(symbolbytes);
	ZeroMemory(symbolbytes, sizeof(symbolbytes));
	SymbolInfo->MaxNameLength = NAME_BUFFER_LEN;
	SymbolInfo->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
    g_debugLoggerVerbose << "----- AnnotateStack ----- " << dec << frameCounter << endl;
	DWORD64 displacement = 0;
	DWORD64 baseAddr = SymGetModuleBase64(m_currentProcess, addr64);
	if (baseAddr == 0)
	{
		// A small problem with symbols
		// They are loaded in the background
		// Sometimes they are not available even if they exist.
		// So I tried to manually load them
		// It works, but there is an easier solution
		// Delay execution a bit if possible and the be available
		// when the background task has finished loaded them

		// The call to ForecLoad has been removed.

		ForceLoad(addr64);
		baseAddr = SymGetModuleBase64(m_currentProcess, addr64);
	}

    g_debugLoggerVerbose << "Addr64 = 0x" << hex << addr64 << " BaseAddr = 0x" << hex << baseAddr;

	ZeroMemory(&ModuleInfo, sizeof(ModuleInfo));
	ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);
	BOOL bRetModuleInfo = SymGetModuleInfo64(m_currentProcess, baseAddr, &ModuleInfo );
	std::string ModuleName;
	if( !bRetModuleInfo ) 
	{			
		ModuleName = std::string("UnknownModule");
	}
	else
	{
		ModuleName = std::string(ModuleInfo.ModuleName);
        g_debugLoggerVerbose << "ModuleName = " << ModuleInfo.ModuleName << endl;
        g_debugLoggerVerbose << "ModuleBase = 0x" << hex << ModuleInfo.BaseOfImage << endl;
        g_debugLoggerVerbose << "Module SymbolType=" << ConvertModuleType2String(ModuleInfo.SymType) << endl;
	}
	BOOL bRetSymFromAddr = SymGetSymFromAddr64(m_currentProcess, addr64, &displacement, SymbolInfo );
	if (!bRetSymFromAddr)
	{
		sprintf_s<NAME_BUFFER_LEN>(buffer, "Error: SymGetSymFromAddr64() failed. Module = %s Addr64 = 0x%08I64X BaseAddr = 0x%08I64X", ModuleName.c_str(), addr64, baseAddr); 
		result = std::string(buffer);
    	g_debugLoggerVerbose.WriteLine(buffer);
		return S_FALSE;
	}
    g_debugLoggerVerbose << "Displacement =  0x" << hex << displacement << endl;

    DWORD disp = (DWORD)displacement;
	BOOL bRetLineFromAddr = SymGetLineFromAddr64(m_currentProcess, addr64, &disp, &line);
	if(bRetLineFromAddr)
	{               
		sprintf_s<NAME_BUFFER_LEN>(buffer, "%s!%s(%i) : 0x%08I64X", ModuleName.c_str(), SymbolInfo->Name, line.LineNumber, addr64);
		result = std::string(buffer);
	}
	else
	{
		sprintf_s<NAME_BUFFER_LEN>(buffer, "%s!%s(UnknownLine) : 0x%08I64X", ModuleName.c_str(), SymbolInfo->Name, addr64);	
		result = std::string(buffer);
	}
	g_debugLoggerVerbose.WriteLine(result.c_str());

	return S_OK;
}

HRESULT NativeStackwalker::AnnotateStack(std::list<DWORD64>& stackFrames, std::list<std::string>& annotatedStackFrames)
{
	if(!m_sym_is_initialized)
		return S_FALSE;
	std::string symInfo;

	annotatedStackFrames.clear();

	for (std::list<DWORD64>::iterator iter = stackFrames.begin(); iter != stackFrames.end(); iter++)
    {
		DWORD64 addr = *iter;
		HRESULT hr = AnnotateStack(addr, symInfo);
		annotatedStackFrames.push_back(std::string(symInfo));
    }
    return S_OK;
}