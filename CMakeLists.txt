cmake_minimum_required(VERSION 3.23.1)

project(Marble.Framework)

# Configuration options...
option(WINDOWS_COMPATIBILITY_SUPPORT "Enable compatibility with Windows versions up to Windows XP." ON)
option(MARBLE_BUILD_TESTS "Build various tests and examples." ON)
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
    # TODO: This looks like garbage. Make it better.
    if (WINDOWS_COMPATIBILITY_SUPPORT)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            message(WARNING "Windows compatibility up to Windows XP is not supported with MSVC.")
            add_definitions(-D_WIN32_WINNT=0x0601)
        else()
            add_definitions(-D_WIN32_WINNT=0x0501 -D__REQUIRED_RPCNDR_H_VERSION__=475)
        endif()
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

set(CMAKE_CXX_STANDARD 23)
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
    add_compile_options(/Zc:__cplusplus)
else()
    set(COMPILER ${CMAKE_CXX_COMPILER_ID})
    set(BUILD_TYPE "UnknownBuildType")
    set(OUTNAME "${BUILD_ARCH}-Unknown-${BUILD_PLATFORM}")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

# SDL2
set(SDL_STATIC_PIC ON CACHE INTERNAL "" FORCE)
set(SDL_SHARED OFF CACHE INTERNAL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/SDL")
target_compile_definitions(SDL2-static PRIVATE HAVE_LIBC=1)

# bgfx

if (COMPILER STREQUAL "MinGW")
    add_definitions(-DMINGW_HAS_SECURE_API=0)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "../libglbuild")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "../libglbuild")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../libglbuild")

set(BGFX_LIBRARY_TYPE "STATIC")
option(BGFX_BUILD_EXAMPLES "Build bgfx examples." OFF)
option(BGFX_CONFIG_MULTITHREADED "Build bgfx with multithreaded configuration" ON)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake")

if (COMPILER STREQUAL "MinGW")
    remove_definitions(-DMINGW_HAS_SECURE_API)
endif()

# Marble.Tools
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Marble.Tools")

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
    "${CMAKE_CURRENT_LIST_DIR}/dependencies"
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

add_custom_target(Marble.Runtime.GL.PrecompiledShaders
    COMMAND ${CMAKE_COMMAND}
    -DSHADERC_PATH="$<TARGET_FILE:Marble.ShaderCompiler>"
    -DSHADERC_DIR="$<TARGET_FILE_DIR:Marble.ShaderCompiler>"
    -DSHADER_FILES_DIR="${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.GL/src/Shaders"
    -DBGFX_SHADER_DIR="${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bgfx/src"
    -P "${CMAKE_CURRENT_LIST_DIR}/cmake/PrecompileBuiltinShaders.cmake"
    COMMENT "[Shader Precompile] Pre-build shader precompilation start."
)

file(GLOB_RECURSE GLSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.GL/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.GL/*.cpp"
)

add_library(Marble.Runtime.GL SHARED ${GLSRCS})
add_dependencies(Marble.Runtime.GL Marble.Runtime.GL.PrecompiledShaders)

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
    bimg
    bx
    bgfx
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
target_link_libraries(Marble.Runtime.CoreLib PUBLIC Marble.Mathematics)
target_link_libraries(Marble.Runtime.CoreLib PRIVATE
    Marble.Runtime.GL
    SDL2-static
)
if (COMPILER STREQUAL "MinGW" OR COMPILER STREQUAL "Clang")
    target_include_directories(Marble.Runtime.CoreLib PRIVATE "${CMAKE_CURRENT_LIST_DIR}/external/bgfx.cmake/bx/include/compat/mingw")
    target_compile_definitions(Marble.Runtime.CoreLib PRIVATE __GXX_TYPEINFO_EQUALITY_INLINE=0)
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

# Marble.Runtime.Components.Core2D

file(GLOB_RECURSE ENGINESRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.Components.Core2D/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.Components.Core2D/*.cpp"
)

add_library(Marble.Runtime.Components.Core2D SHARED ${ENGINESRCS})

generate_export_header(Marble.Runtime.Components.Core2D
    BASE_NAME MARBLE_COMPONENTCORE
    EXPORT_MACRO_NAME __marble_componentcore_api
    EXPORT_FILE_NAME Marble.Runtime.Components.Core2D.Exports.h
)

target_include_directories(Marble.Runtime.Components.Core2D PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Typography/src"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.GL/src"
    "${CMAKE_CURRENT_LIST_DIR}/Marble.Runtime.Components.Core2D/src"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/earcut/include"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/stb"
)
target_link_libraries(Marble.Runtime.Components.Core2D PRIVATE
    Marble.Mathematics
    Marble.Typography
    Marble.Runtime.GL
    Marble.Runtime.CoreLib
)

if (MARBLE_ENABLE_PROFILING)
    target_link_libraries(Marble.Runtime.Components.Core2D PRIVATE TracyClient)
endif()

set_target_properties(Marble.Runtime.Components.Core2D
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Runtime.Components.Core2D"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Runtime.Components.Core2D"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${OUTNAME}/Marble.Runtime.Components.Core2D"
)
set_target_properties(Marble.Runtime.Components.Core2D
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Runtime.Components.Core2D"
    LIBRARY_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Marble.Runtime.Components.Core2D"
    RUNTIME_OUTPUT_DIRECTORY "../bin/${OUTNAME}/Marble.Runtime.Components.Core2D"
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

if (MARBLE_BUILD_TESTS)
    add_executable(VisualTest.Marble.RectTransform "${CMAKE_CURRENT_LIST_DIR}/Marble.Tests/VisualTest.Marble.RectTransform/main.cpp")
    target_link_libraries(VisualTest.Marble.RectTransform PUBLIC Marble.Runtime.Components.Core2D Marble.Runtime.CoreLib)
    set_target_properties(VisualTest.Marble.RectTransform
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Tests"
        LIBRARY_OUTPUT_DIRECTORY "../lib/${OUTNAME}/Tests"
        RUNTIME_OUTPUT_DIRECTORY "../bin/${OUTNAME}/Tests"
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden
    )

    add_custom_command(TARGET VisualTest.Marble.RectTransform POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Marble.Typography>
        $<TARGET_FILE_DIR:VisualTest.Marble.RectTransform>
    )
    add_custom_command(TARGET VisualTest.Marble.RectTransform POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Marble.Runtime.GL>
        $<TARGET_FILE_DIR:VisualTest.Marble.RectTransform>
    )
    add_custom_command(TARGET VisualTest.Marble.RectTransform POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Marble.Runtime.CoreLib>
        $<TARGET_FILE_DIR:VisualTest.Marble.RectTransform>
    )
    add_custom_command(TARGET VisualTest.Marble.RectTransform POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Marble.Runtime.Components.Core2D>
        $<TARGET_FILE_DIR:VisualTest.Marble.RectTransform>
    )
endif()

if (BUILD_PLATFORM STREQUAL "Win32")
    remove_definitions(-D_WIN32_WINNT)
endif()
