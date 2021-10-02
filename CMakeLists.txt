cmake_minimum_required(VERSION 3.13.4)

project(Marble.Framework)

# Configuration options...
option(WINDOWS_COMPATIBILITY_SUPPORT "Enable compatibility with Windows versions up to Windows XP." ON)
option(MARBLE_ENABLE_PROFILING "Enable frame-by-frame profiling with tracy." OFF)

include(GenerateExportHeader)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BUILD_ARCH x64)
else()
    set(BUILD_ARCH x86)
endif()

if ((NOT DEFINED CMAKE_BUILD_TYPE) OR (CMAKE_BUILD_TYPE STREQUAL ""))
    set(CMAKE_BUILD_TYPE "Release")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG=1)
endif()

if (WIN32 OR WIN64)
    set(BUILD_PLATFORM Win32)
    if (WINDOWS_COMPATIBILITY_SUPPORT)
        add_definitions(-D_WIN32_WINNT=0x0501)
    else()
        add_definitions(-D_WIN32_WINNT=0x0601)
    endif()
elseif (UNIX AND NOT APPLE)
    set(BUILD_PLATFORM Linux)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
elseif (APPLE)
    set(BUILD_PLATFORM MacOS)
else()
    set(BUILD_PLATFORM "UnknownPlatform")
endif()

set(CMAKE_CXX_STANDARD 20)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_SHARED_LIBRARY_SUFFIX STREQUAL ".dll")
        set(COMPILER "MinGW")
    else()
        set(COMPILER "gcc")
    endif()
    set(BUILD_TYPE "${CMAKE_BUILD_TYPE}")
    set(OUTNAME "${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}-${BUILD_PLATFORM}")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(COMPILER "Clang")
    set(BUILD_TYPE "${CMAKE_BUILD_TYPE}")
    set(OUTNAME "${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}-${BUILD_PLATFORM}")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(COMPILER "MSVC")
    set(BUILD_TYPE "Any")
    set(OUTNAME "${BUILD_ARCH}-${COMPILER}-${BUILD_PLATFORM}")
else()
    set(COMPILER ${CMAKE_CXX_COMPILER_ID})
    set(BUILD_TYPE "UnknownBuildType")
    set(OUTNAME "${BUILD_ARCH}-Unknown-${BUILD_PLATFORM}")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

# Marble.Packager
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Marble.Packager")

# SDL2
set(SDL_STATIC_PIC ON CACHE INTERNAL "" FORCE)
set(SDL_SHARED OFF CACHE INTERNAL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/SDL")
target_compile_definitions(SDL2-static PRIVATE HAVE_LIBC=1)

# bgfx + Dependencies
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/libgl")

# tracy
if (MARBLE_ENABLE_PROFILING)
    if (WINDOWS_COMPATIBILITY_SUPPORT)
        message(WARNING "Profiling cannot be used with Windows compatibility support.")
    else()
        set(TRACY_CALLSTACK ON CACHE INTERNAL "" FORCE)
        add_definitions(-DMARBLE_ENABLE_PROFILING -DTRACY_ENABLE)
        add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/tracy")
        target_link_libraries(TracyClient PRIVATE ws2_32 dbghelp)
    endif()
endif()

# Marble.Mathematics

add_library(Marble.Mathematics INTERFACE)
target_include_directories(Marble.Mathematics INTERFACE "${CMAKE_CURRENT_LIST_DIR}/Marble.Mathematics/src")

# Marble.Typography

file(GLOB_RECURSE FONTSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Typography/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Typography/*.cpp"
)

add_library(Marble.Typography SHARED ${FONTSRCS})

generate_export_header(Marble.Typography
    BASE_NAME MARBLE_TYPOGRAPHY
    EXPORT_MACRO_NAME __marble_typography_api
    EXPORT_FILE_NAME Marble.Typography.Exports.h
)

target_include_directories(Marble.Typography PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Typography/src"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/earcut/include"
)
target_include_directories(Marble.Typography PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.CoreLib/src"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/stb"
)

if (MARBLE_ENABLE_PROFILING)
    target_link_libraries(Marble.Typography PRIVATE TracyClient)
endif()

set_target_properties(Marble.Typography
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Typography"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Typography"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${OUTNAME}/Marble.Typography"
)

# Marble.Runtime.GL

file(GLOB_RECURSE GLSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.GL/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.GL/*.cpp"
)
file(GLOB_RECURSE SHADERC configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/external/libglcompilers/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/external/libglcompilers/*.cpp"
)

add_library(Marble.Runtime.GL SHARED ${GLSRCS} ${SHADERC})

generate_export_header(Marble.Runtime.GL
    BASE_NAME MARBLE_GL
    EXPORT_MACRO_NAME __marble_gl_api
    EXPORT_FILE_NAME Marble.Runtime.GL.Exports.h
)

target_include_directories(Marble.Runtime.GL PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.GL/src"
)
target_include_directories(Marble.Runtime.GL PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.CoreLib/src"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies"
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bgfx/include"
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include"
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bimg/include"
)
target_link_libraries(Marble.Runtime.GL PRIVATE
    gl
)
if (COMPILER STREQUAL "MinGW" OR COMPILER STREQUAL "Clang")
    target_include_directories(Marble.Runtime.GL PRIVATE "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include/compat/mingw")
elseif (COMPILER STREQUAL "MSVC")
    target_include_directories(Marble.Runtime.GL PRIVATE "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include/compat/msvc")
else()
endif()

if (MARBLE_ENABLE_PROFILING)
    target_link_libraries(Marble.Runtime.GL PRIVATE TracyClient)
endif()

set_target_properties(Marble.Runtime.GL
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Runtime.GL"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Runtime.GL"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${OUTNAME}/Marble.Runtime.GL"
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

# Marble.Runtime.CoreLib

file(GLOB_RECURSE ENGINESRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.CoreLib/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.CoreLib/*.cpp"
)

add_library(Marble.Runtime.CoreLib SHARED ${ENGINESRCS})

generate_export_header(Marble.Runtime.CoreLib
    BASE_NAME MARBLE_CORELIB
    EXPORT_MACRO_NAME __marble_corelib_api
    EXPORT_FILE_NAME Marble.Runtime.CoreLib.Exports.h
)

target_compile_definitions(Marble.Runtime.CoreLib PUBLIC SDL_MAIN_HANDLED)
target_include_directories(Marble.Runtime.CoreLib PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.CoreLib/src"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.CoreLib/include"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/robin_hood/include"
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bgfx/include"
    "${CMAKE_CURRENT_LIST_DIR}/external/SDL/include"
)
target_include_directories(Marble.Runtime.CoreLib PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include"
)
target_link_libraries(Marble.Runtime.CoreLib PRIVATE
    Marble.Mathematics
    Marble.Runtime.GL
    SDL2-static
)
if (COMPILER STREQUAL "MinGW" OR COMPILER STREQUAL "Clang")
    target_include_directories(Marble.Runtime.CoreLib PRIVATE "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include/compat/mingw")
elseif (COMPILER STREQUAL "MSVC")
    target_include_directories(Marble.Runtime.CoreLib PRIVATE "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include/compat/msvc")
else()
endif()
if (BUILD_PLATFORM STREQUAL Win32)
    target_link_libraries(Marble.Runtime.CoreLib PRIVATE psapi)
endif()

if (MARBLE_ENABLE_PROFILING)
    target_link_libraries(Marble.Runtime.CoreLib PRIVATE TracyClient)
endif()

set_target_properties(Marble.Runtime.CoreLib
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Runtime.CoreLib"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Runtime.CoreLib"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${OUTNAME}/Marble.Runtime.CoreLib"
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

# Marble.Runtime.Components.Core

file(GLOB_RECURSE ENGINESRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.Components.Core/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.Components.Core/*.cpp"
)

add_library(Marble.Runtime.Components.Core SHARED ${ENGINESRCS})

generate_export_header(Marble.Runtime.Components.Core
    BASE_NAME MARBLE_COMPONENTCORE
    EXPORT_MACRO_NAME __marble_componentcore_api
    EXPORT_FILE_NAME Marble.Runtime.Components.Core.Exports.h
)

target_include_directories(Marble.Runtime.Components.Core PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Typography/src"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.Components.Core/src"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/stb"
)
target_link_libraries(Marble.Runtime.Components.Core PRIVATE
    Marble.Mathematics
    Marble.Typography
    Marble.Runtime.GL
    Marble.Runtime.CoreLib
)

if (MARBLE_ENABLE_PROFILING)
    target_link_libraries(Marble.Runtime.Components.Core PRIVATE TracyClient)
endif()

set_target_properties(Marble.Runtime.Components.Core
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Runtime.Components.Core"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Runtime.Components.Core"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${OUTNAME}/Marble.Runtime.Components.Core"
)
set_target_properties(Marble.Runtime.Components.Core
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Runtime.Components.Core"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Runtime.Components.Core"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${OUTNAME}/Marble.Runtime.Components.Core"
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

if (BUILD_PLATFORM STREQUAL "Win32")
    remove_definitions(-D_WIN32_WINNT)
endif()
