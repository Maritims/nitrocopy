# We're building for Windows..
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)

# ..with MinGW cross compiler
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)

# ..for 32-bit and targeting Win9x
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -D_WIN32_WINNT=0x0400 -Wl,--subsystem,console:4.0,-lmsvcrt -DNITRO_DEBUG")

set(CMAKE_FIND_ROOT_PATH "/usr/i686-w64-mingw32")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
