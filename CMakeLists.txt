cmake_minimum_required(VERSION 3.25)

# C is enabled for one reason: the Program ABI is a C header, and compiling it as C17 once is what
# proves the vendored copy is the Program ABI and not a C++ reading of it.
project(RcWorm
    LANGUAGES CXX C
    VERSION 0.1.0
    DESCRIPTION "A remote-controlled glass-neon mecha-worm which plugs into TronGrid Lite as a Program"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# The panel is the one place Qt lives. Off, the worm still builds and loads - a Program without
# its window is a Program that stands still - and CI proves that beside the full build.
option(RC_WORM_PANEL "Build the Qt panel the User steers from" ON)

option(RC_WORM_ENABLE_CLANG_TIDY "Run clang-tidy as part of the build" OFF)

if(RC_WORM_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    message(STATUS "clang-tidy enabled: ${CLANG_TIDY_EXE}")
endif()

# Compiler warnings - high level, warnings as errors, all compilers: the flagship's policy.
if(MSVC)
    add_compile_options(/W4 /WX /utf-8 $<$<COMPILE_LANGUAGE:CXX>:/EHsc>)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

if(NOT (WIN32 OR (UNIX AND NOT APPLE)))
    message(FATAL_ERROR "Unsupported platform. Only Windows and Linux are supported.")
endif()

# A Program's file name is what the Grid loads by: `rc_worm.dll` on Windows, `librc_worm.so` on
# Linux - the same rule the flagship's own fixtures follow.
if(WIN32)
    set(RC_WORM_LIBRARY_PREFIX "")
else()
    set(RC_WORM_LIBRARY_PREFIX "lib")
endif()

# One runtime directory per configuration, so the tests find the library beside them the way the
# Grid finds a Program beside itself - and a MinGW build's import library beside the MSVC build's.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

enable_testing()

add_subdirectory(libs)
add_subdirectory(src)
