REM Microsoft CORProfilerAPI
SET COR_ENABLE_PROFILING=1
SET COR_PROFILER={C6DBEE4B-017D-43AC-8689-3B107A6104EF}
SET COR_PROFILER_PATH=D:\junFiles\IProfiler\MixedModeProfiler_src\MixedModeProfiler_src\IProfiler\IProfiler\MixedModeProfiler_src\MixedModeProfiler_src\DiagProfiler\Debug\DiagProfiler.dll

REM CLR Option
SET COMPLUS_Version=v4.0.30319

REM DiagProfiler
SET DIAG_PRF_SYMBOLPATH=SRV*c:\symbols*http://msdl.microsoft.com/download/symbols;D:\junFiles\IProfiler\MixedModeProfiler_src\MixedModeProfiler_src\IProfiler\IProfiler\MixedModeProfiler_src\MixedModeProfiler_src\DiagProfiler\Debug;
SET DIAG_PRF_DEBUGTRACE=diag_debugtrace.txt
SET DIAG_PRF_STACKTRACE=diag_stacktrace.txt
SET DIAG_PRF_VERBOSE=verbose.txt
SET DIAG_PRF_ONLY_MANAGED_THREADS=1
@REM START windbg.exe
START CppCliApp.exe