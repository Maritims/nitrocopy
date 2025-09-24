cmake_minimum_required(VERSION 3.12)
project(NitroCopy
    VERSION     0.0.1
    LANGUAGES   C
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------------------------
# Determine Git-based version
# -----------------------------------------------
execute_process(
    COMMAND git describe --tags --always
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Fallback to project version if Git info is unavailable
if(NOT GIT_VERSION)
    set(GIT_VERSION ${PROJECT_VERSION})
endif()

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
# Determine Git version
# -----------------------------------------------
execute_process(
    COMMAND git describe --tags --always
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT GIT_VERSION)
    set(GIT_VERSION ${PROJECT_VERSION})
endif()

# -----------------------------------------------
# Generate version header
# -----------------------------------------------
# Path to build number file
set(BUILD_NUMBER_FILE "${CMAKE_BINARY_DIR}/build_number.txt")

# Initialize file if it doesn't exist
if(NOT EXISTS "${BUILD_NUMBER_FILE}")
    file(WRITE "${BUILD_NUMBER_FILE}" "0")
endif()

# Read current build number
file(READ "${BUILD_NUMBER_FILE}" BUILD_NUMBER)
string(STRIP "${BUILD_NUMBER}" BUILD_NUMBER)

# Increment build number
math(EXPR BUILD_NUMBER "${BUILD_NUMBER} + 1")

# Write back new build number
file(WRITE "${BUILD_NUMBER_FILE}" "${BUILD_NUMBER}")

# Export as a cache variable for configure_file
set(NITRO_BUILD_NUMBER "${BUILD_NUMBER}" CACHE INTERNAL "Current build number")
message(STATUS "Build number: ${BUILD_NUMBER}")

set(VERSION_STRING "${GIT_VERSION} (build ${NITRO_BUILD_NUMBER})")

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
add_subdirectory(src)

# -----------------------------------------------
# Custom target for tagging releases
# -----------------------------------------------
add_custom_target(tag_release
    COMMAND git tag -a v${PROJECT_VERSION} -m "Release v${PROJECT_VERSION}"
    COMMAND git push --tags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Tagging release v${PROJECT_VERSION}"
)

# -----------------------------------------------
# Tests
# -----------------------------------------------
#include(CTest)
#enable_testing()

# Add tests subdirectory
#add_subdirectory(tests)
