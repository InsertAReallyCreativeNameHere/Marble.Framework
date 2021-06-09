cmake_minimum_required(VERSION 3.10)

project(Marble.Framework)

if (WIN32 OR WIN64)
    set(BUILD_PLATFORM Win32)
elseif (UNIX AND NOT APPLE)
    set(BUILD_PLATFORM Linux)
else()
    set(BUILD_PLATFORM MacOS)
endif()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BUILD_ARCH x64)
else()
    set(BUILD_ARCH x86)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_SHARED_LIBRARY_PREFIX "")

set(CMAKE_CXX_STANDARD 20)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_SHARED_LIBRARY_SUFFIX STREQUAL ".dll")
        set(COMPILER "MinGW")
    else()
        set(COMPILER "gcc")
    endif()
    set(BUILD_TYPE "${CMAKE_BUILD_TYPE}-" CACHE STRING "Configuration string in output dir." FORCE)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(COMPILER "Clang")
    set(BUILD_TYPE "${CMAKE_BUILD_TYPE}-" CACHE STRING "Configuration string in output dir." FORCE)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(COMPILER "MSVC")
    set(BUILD_TYPE "")
else()
    set(COMPILER ${CMAKE_CXX_COMPILER_ID})
endif()

if (DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG=1)
endif()

set(SDL_SHARED OFF CACHE BOOL "Build SDL2 shared library." FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/SDL")
target_compile_definitions(SDL2-static PRIVATE HAVE_LIBC=1)

set(_CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})
set(CMAKE_BUILD_TYPE "Release") # Builds faster for MinGW/GCC this way.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../libglbuild)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../libglbuild)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../libglbuild)
set(BGFX_BUILD_TOOLS OFF CACHE BOOL "Build bgfx tools." FORCE)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "Build bgfx examples." FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/libgl")
target_compile_definitions(bgfx PRIVATE BGFX_CONFIG_MULTITHREADED=0)
set(CMAKE_BUILD_TYPE ${_CMAKE_BUILD_TYPE})

# Marble.Mathematics

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Marble.Mathematics)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Marble.Mathematics)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Marble.Mathematics)

file(GLOB_RECURSE METHSRCS configure_depends
    "${CMAKE_SOURCE_DIR}/Marble.Mathematics/*.h"
    "${CMAKE_SOURCE_DIR}/Marble.Mathematics/*.cpp"
)

add_library(Marble.Mathematics SHARED ${METHSRCS})
target_compile_definitions(Marble.Mathematics PRIVATE BUILD_TYPE_DYNAMIC=1)

if (${BUILD_PLATFORM} STREQUAL Win32)
    target_link_libraries(Marble.Mathematics PRIVATE
    )
    target_include_directories(Marble.Mathematics PRIVATE
        "${CMAKE_SOURCE_DIR}/Marble.Mathematics/src"
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
    )
elseif (${BUILD_PLATFORM} STREQUAL Linux)
    target_link_libraries(Marble.Mathematics PRIVATE
    )
    target_include_directories(Marble.Mathematics PRIVATE
        "${CMAKE_SOURCE_DIR}/Marble.Mathematics/src"
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
    )
endif()

# Marble.Typography

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Marble.Typography)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Marble.Typography)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Marble.Typography)

file(GLOB_RECURSE FONTSRCS configure_depends
    "${CMAKE_SOURCE_DIR}/Marble.Typography/*.h"
    "${CMAKE_SOURCE_DIR}/Marble.Typography/*.cpp"
)

add_library(Marble.Typography SHARED ${FONTSRCS})
target_compile_definitions(Marble.Typography PRIVATE BUILD_TYPE_DYNAMIC=1)

target_link_libraries(Marble.Typography PRIVATE
)
target_include_directories(Marble.Typography PRIVATE
    "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
    "${CMAKE_SOURCE_DIR}/Marble.Typography/src"
    "${CMAKE_SOURCE_DIR}/dependencies/stb"
)

# Marble.Runtime.GL

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Marble.Runtime.GL)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Marble.Runtime.GL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Marble.Runtime.GL)

file(GLOB_RECURSE GLSRCS configure_depends
    "${CMAKE_SOURCE_DIR}/Marble.Runtime.GL/*.h"
    "${CMAKE_SOURCE_DIR}/Marble.Runtime.GL/*.cpp"
)
file(GLOB_RECURSE SHADERC configure_depends
    "${CMAKE_SOURCE_DIR}/external/libglcompilers/*.h"
    "${CMAKE_SOURCE_DIR}/external/libglcompilers/*.cpp"
)

add_library(Marble.Runtime.GL SHARED ${GLSRCS} ${SHADERC})
target_compile_definitions(Marble.Runtime.GL PRIVATE BUILD_TYPE_DYNAMIC=1)

if (${BUILD_PLATFORM} STREQUAL Win32)
    target_include_directories(Marble.Runtime.GL PRIVATE
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.GL/src"
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
        "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bgfx/include"
        "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bx/include"
        "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bimg/include"
    )
    target_link_libraries(Marble.Runtime.GL PRIVATE
        gl
    )
    if (COMPILER STREQUAL "MinGW")
        target_include_directories(Marble.Runtime.GL PRIVATE "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bx/include/compat/mingw")
    elseif (COMPILER STREQUAL "MSVC")
        target_include_directories(Marble.Runtime.GL PRIVATE "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bx/include/compat/msvc")
    else()
    endif()
elseif (${BUILD_PLATFORM} STREQUAL Linux)
    target_link_libraries(Marble.Runtime.GL PRIVATE
    )
    target_include_directories(Marble.Runtime.GL PRIVATE
    )
endif()

# Marble.Runtime.CoreLib

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Marble.Runtime.CoreLib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Marble.Runtime.CoreLib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Marble.Runtime.CoreLib)

file(GLOB_RECURSE ENGINESRCS configure_depends
    "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/*.h"
    "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/*.cpp"
)

add_library(Marble.Runtime.CoreLib SHARED ${ENGINESRCS})
target_compile_definitions(Marble.Runtime.CoreLib PRIVATE SDL_MAIN_HANDLED BUILD_TYPE_DYNAMIC=1)

target_include_directories(Marble.Runtime.CoreLib PRIVATE
    "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
    "${CMAKE_SOURCE_DIR}/Marble.Runtime.GL/src"
    "${CMAKE_SOURCE_DIR}/Marble.Mathematics/src"
    "${CMAKE_SOURCE_DIR}/dependencies/ctti/include"
    "${CMAKE_SOURCE_DIR}/dependencies/stb"
    "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bgfx/include" # Do not attempt to use bgfx in Marble.Runtime.CoreLib, this exists as a dependency of other headers.
    "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bx/include"
    "${CMAKE_SOURCE_DIR}/external/SDL2/include"
)
target_link_libraries(Marble.Runtime.CoreLib PRIVATE
    Marble.Mathematics
    Marble.Runtime.GL
    SDL2-static
)
if (${BUILD_PLATFORM} STREQUAL Win32)
    target_include_directories(Marble.Runtime.CoreLib PRIVATE
    )
    if (COMPILER STREQUAL "MinGW")
        target_include_directories(Marble.Runtime.CoreLib PRIVATE "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bx/include/compat/mingw")
    elseif (COMPILER STREQUAL "MSVC")
        target_include_directories(Marble.Runtime.CoreLib PRIVATE "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bx/include/compat/msvc")
    else()
    endif()
    target_link_libraries(Marble.Runtime.CoreLib PRIVATE
        psapi
    )
elseif (${BUILD_PLATFORM} STREQUAL Linux)
    target_link_libraries(Marble.Runtime.CoreLib PRIVATE
        ${CMAKE_DL_LIBS}
    )
    target_include_directories(Marble.Runtime.CoreLib PRIVATE
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
    )
endif()

# Sandbox

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Sandbox)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Sandbox)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Sandbox)

file(GLOB_RECURSE SANDBOXSRCS configure_depends
    "${CMAKE_SOURCE_DIR}/Sandbox/src/*.h"
    "${CMAKE_SOURCE_DIR}/Sandbox/src/*.cpp"
)

add_executable(Sandbox ${SANDBOXSRCS})
add_dependencies(Sandbox Marble.Runtime.CoreLib)
target_compile_definitions(Sandbox PRIVATE SDL_MAIN_HANDLED BUILD_TYPE_DYNAMIC=0)

if (${BUILD_PLATFORM} STREQUAL Win32)
    target_link_libraries(Sandbox PRIVATE
        Marble.Mathematics
        Marble.Typography
        Marble.Runtime.CoreLib
    )
    target_include_directories(Sandbox PRIVATE
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/include"
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.GL/src"
        "${CMAKE_SOURCE_DIR}/Marble.Typography/src"
        "${CMAKE_SOURCE_DIR}/Marble.Mathematics/src"
        "${CMAKE_SOURCE_DIR}/dependencies/ctti/include"
        "${CMAKE_SOURCE_DIR}/dependencies/stb"
        "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bgfx/include" # Do not attempt to use bgfx in Marble.Runtime.CoreLib, this exists as a dependency of other headers.
        "${CMAKE_SOURCE_DIR}/external/SDL/include"
    )

    add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Marble.Mathematics>
        $<TARGET_FILE_DIR:Sandbox>
    )
    add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Marble.Typography>
        $<TARGET_FILE_DIR:Sandbox>
    )
    add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Marble.Runtime.GL>
        $<TARGET_FILE_DIR:Sandbox>
    )
    add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Marble.Runtime.CoreLib>
        $<TARGET_FILE_DIR:Sandbox>
    )
elseif (${BUILD_PLATFORM} STREQUAL Linux)
    target_link_libraries(Sandbox PRIVATE
    )
    target_include_directories(Sandbox PRIVATE
    )
endif()
