There are 3 ways to run the sample apps.

DiagProfiler.dll     [Profiler]
1. Update profile.bat to point out directories and diagprofiler.dll
2. Start CppCliApp.exe (demo app)
3. Press enter to close CppCliApp.exe

DiagProfilerConsole  [Profiler console controller app]
Start CppCliApp.exe
Use the PId of CppCliApp to attach a profiler.
DiagProfilerConsole.exe <pId>

DiagProfilerLauncher [Profiler winforms controller app]
1. Start exe / Attach to process 
2. Start [sampling]
3. Wait
4. Stop

Look in the article for further explanations.