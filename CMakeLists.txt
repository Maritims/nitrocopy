cmake_minimum_required(VERSION 3.12)
project(NitroCopy
    VERSION     0.0.1
    LANGUAGES   C
)


# -----------------------------------------------
# Compiler settings
# -----------------------------------------------
set(CMAKE_C_STANDARD 90)            # It's far more convenient to use C90 when also targeting Windows 98.
set(CMAKE_C_STANDARD_REQUIRED ON)   # Enforce the chosen standard.
set(CMAKE_C_EXTENSIONS OFF)         # We want strict C90 mode, so we disable extensions.

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Enable warnings and tream them as errors.
add_compile_options(
    -Wall                   # Enable most common warnings.
    -Wextra                 # Enable less common warnings not included by -Wall.
    -Wstrict-prototypes     # Require functions to have argument types.
    -pedantic               # Enforce strict ISO C90 compliance.
)

# Debug vs release mode
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -DNITRO_DEBUG")  # Disable optimisations (o0) and include debug symbols (-g).
set(CMAKE_C_FLAGS_RELEASE "-O2")                # Optimise for speed.

# -----------------------------------------------
# Include directories
# -----------------------------------------------
include_directories(${CMAKE_SOURCE_DIR}/include)

# -----------------------------------------------
# Generate version header
# -----------------------------------------------
configure_file(
    ${CMAKE_SOURCE_DIR}/include/nitro_version.h.in
    ${CMAKE_BINARY_DIR}/include/nitro_version.h
    @ONLY
)

# Include generated headers
include_directories(${CMAKE_BINARY_DIR}/include)

# -----------------------------------------------
# Source files
# -----------------------------------------------
file(GLOB_RECURSE NITRO_SOURCES "src/*.c")

# -----------------------------------------------
# Build application
# -----------------------------------------------
add_executable(NitroCopy ${NITRO_SOURCES})

# Link libraries.
target_link_libraries(NitroCopy PRIVATE m)

if (WIN32)
    target_link_options(NitroCopy PRIVATE -static -static-libgcc -static-libstdc++)
endif()
