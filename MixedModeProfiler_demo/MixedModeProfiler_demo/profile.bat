REM Microsoft CORProfilerAPI
SET COR_ENABLE_PROFILING=1
SET COR_PROFILER={C6DBEE4B-017D-43AC-8689-3B107A6104EF}
SET COR_PROFILER_PATH=C:\Source\MixedModeProfiler\MixedModeProfiler_demo\DiagProfiler.dll

REM CLR Option
SET COMPLUS_Version=v4.0.30319

REM DiagProfiler
SET DIAG_PRF_SYMBOLPATH=SRV*c:\symbols*http://msdl.microsoft.com/download/symbols;
SET DIAG_PRF_DEBUGTRACE=C:\temp\diag_debugtrace.txt
SET DIAG_PRF_STACKTRACE=C:\temp\diag_stacktrace.txt
SET DIAG_PRF_ONLY_MANAGED_THREADS=1
