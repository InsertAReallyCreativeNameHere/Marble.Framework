cmake_minimum_required(VERSION 3.13.4)

project(Marble.Framework)

# Configuration options...
set(MARBLE_ENABLE_PROFILING OFF CACHE BOOL "Enable frame-by-frame profiling with tracy.")

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
    add_definitions(-D_WIN32_WINNT=0x0501)
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
set(SDL_STATIC_PIC ON)
set(SDL_SHARED OFF)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/SDL")
target_compile_definitions(SDL2-static PRIVATE HAVE_LIBC=1)

# bgfx + Dependencies
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/libgl")

# tracy
if (MARBLE_ENABLE_PROFILING)
    set(TRACY_CALLSTACK ON)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/tracy")
    add_definitions(-DMARBLE_ENABLE_PROFILING)
endif()

# Marble.Mathematics

file(GLOB_RECURSE METHSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Mathematics/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Mathematics/*.cpp"
)

add_library(Marble.Mathematics SHARED ${METHSRCS})
target_compile_definitions(Marble.Mathematics PRIVATE BUILD_TYPE_DYNAMIC=1)
target_include_directories(Marble.Mathematics PUBLIC "${CMAKE_CURRENT_LIST_DIR}/Marble.Mathematics/src")
target_include_directories(Marble.Mathematics PRIVATE "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.CoreLib/src")
if (MARBLE_ENABLE_PROFILING)
    target_link_libraries(Marble.Mathematics PRIVATE TracyClient)
endif()

set_target_properties(Marble.Mathematics
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Mathematics"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Mathematics"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${OUTNAME}/Marble.Mathematics"
)

# Marble.Typography

file(GLOB_RECURSE FONTSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Typography/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Typography/*.cpp"
)

add_library(Marble.Typography SHARED ${FONTSRCS})
target_compile_definitions(Marble.Typography PRIVATE BUILD_TYPE_DYNAMIC=1)

target_include_directories(Marble.Typography PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Typography/src"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/earcut/include"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/stb"
)
target_include_directories(Marble.Typography PRIVATE "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.CoreLib/src")

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
target_compile_definitions(Marble.Runtime.GL PRIVATE BUILD_TYPE_DYNAMIC=1)

target_include_directories(Marble.Runtime.GL PUBLIC "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.GL/src")
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
)

# Marble.Runtime.CoreLib

file(GLOB_RECURSE ENGINESRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.CoreLib/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.CoreLib/*.cpp"
)

add_library(Marble.Runtime.CoreLib SHARED ${ENGINESRCS})

target_compile_definitions(Marble.Runtime.CoreLib PUBLIC SDL_MAIN_HANDLED)
target_compile_definitions(Marble.Runtime.CoreLib PRIVATE BUILD_TYPE_DYNAMIC=1)

target_include_directories(Marble.Runtime.CoreLib PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Mathematics/src"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Typography/src"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.GL/src"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.CoreLib/src"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.CoreLib/include"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/earcut/include"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/robin_hood/include"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/stb"
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bgfx/include"
    "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include"
    "${CMAKE_CURRENT_LIST_DIR}/external/SDL/include"
)
target_link_libraries(Marble.Runtime.CoreLib PRIVATE
    Marble.Mathematics
    Marble.Typography
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
)
