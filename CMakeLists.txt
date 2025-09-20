cmake_minimum_required(VERSION 3.12)
project(NitroCopy C)

# -----------------------------------------------
# Compiler settings
# -----------------------------------------------
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Enable warnings and tream them as errors.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c90 -pedantic -Wstrict-prototypes -Wall -Wextra -DNITRO_DEBUG")

# -----------------------------------------------
# Include directories
# -----------------------------------------------
include_directories(${CMAKE_SOURCE_DIR}/include)

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
