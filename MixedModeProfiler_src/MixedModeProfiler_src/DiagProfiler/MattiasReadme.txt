DiagProfiler         [Profiler]
1. Update profile.bat to point out directories and diagprofiler.dll
2. Start CppCliApp.exe (demo app)
3. Press enter to close CppCliApp.exe

DiagProfilerClient   [Library]

DiagProfilerConsole  [Profiler console controller app]
DiagProfilerConsole.exe <pId>

DiagProfilerLauncher [Profiler winforms controller app]
1. Start exe / Attach to process 
2. Start [sampling]
3. Wait
4. Stop

----------- Comments ----
With COM and console less applications,
it is no standard way to return error codes.
I used OutputDebugString for the startup code.

I recommend using Debugview (need administrator privileges)
http://technet.microsoft.com/en-us/sysinternals/bb896647


There is a small problem with AttachProfiler
When we attach afterwards, we will have missed the CLR callbacks
telling us which managed thread ids corresponds to which win32 thread ids.
With out that info, we cannot call DoStackSnapshot().
This means that we can not get the managed stackwalk
It will treat it as a native thread.

One idea I had, which worked partially,
was to connect from DiagProfilerClient with a debugger,
and list the managed threads, and then feed the command server.
I managed to list the managed threads though ICorDebug
But the enumeration of those threads only gave me the win32 thread Id.

The best way to profile is probably to use the command line
or the DiagProfilerLauncher, which implicitely uses the command line
to make the profiler start in a suspended mode.

I am not content with the logger part.
My first implementation used fprintf(FILE*)
It worked perfectly. But is is not typesafe.
I "improved" it by switching to streams. Perhaps a bad idea.
I got crashes due to simultaneous writes from different threads

g_stackLogger is only used by the OnSampleThread
g_debugLogger is used by all threads

I therefore had to implement LockedStream

Using locks is a really bad idea out of performance reasons!!
The best way is probably to to let the startup code and the command server
use OutputDebugString or its own logfile. Then the lock can be removed,
because there is only 1 thread that will be accessing the logger.

Another point of optimization in my code
is the heavy use of std::string and std::wstring
In my first version I wanted to avoid dynamic allocations.
The strings are copied several times.
Using more references or pointers this can probably be avoided.

Mattias Högström