# Toolchain for Windows 9x (MinGW-w64, 32-bit)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)

# Compiler
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)

# Win9x compatibility
add_compile_options(
    -D_WIN32_WINNT=0x0400 # Target Win98 / NT 4.0
)

# Force console subsystem and link against MSVCRT
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--subsystem,console:4.0,-lmsvcrt")

# Search paths
set(CMAKE_FIND_ROOT_PATH "/usr/i686-w64-mingw32")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
