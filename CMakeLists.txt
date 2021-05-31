cmake_minimum_required(VERSION 3.10)

if (WIN32)
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

project(Marble.Framework)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_SHARED_LIBRARY_PREFIX "")

set(CMAKE_CXX_STANDARD 20)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_SHARED_LIBRARY_SUFFIX STREQUAL ".dll")
        set(COMPILER "MinGW")
    else()
        set(COMPILER "gcc")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(COMPILER "Clang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(COMPILER "MSVC")
else()
    set(COMPILER ${CMAKE_CXX_COMPILER_ID})
endif()

set(SDL_SHARED OFF)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/SDL")
target_compile_definitions(SDL2-static PUBLIC SDL_RENDER_DISABLED=1)

# It builds faster this way
set(PREV_BUILD_TYPE ${CMAKE_BUILD_TYPE})
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../libglbuild)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../libglbuild)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../libglbuild)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/libgl")
target_compile_definitions(bgfx PUBLIC BGFX_CONFIG_MULTITHREADED=0)
set(CMAKE_BUILD_TYPE ${PREV_BUILD_TYPE})

# Marble.Mathematics

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Mathematics)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Mathematics)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Mathematics)

file(GLOB_RECURSE METHSRCS configure_depends
    "${CMAKE_SOURCE_DIR}/Marble.Mathematics/*.h"
    "${CMAKE_SOURCE_DIR}/Marble.Mathematics/*.cpp"
)

add_library(Marble.Mathematics SHARED ${METHSRCS})
target_compile_definitions(Marble.Mathematics PUBLIC BUILD_TYPE_DYNAMIC=1)

if (${BUILD_PLATFORM} STREQUAL Win32)
    target_link_libraries(Marble.Mathematics PUBLIC
    )
    target_include_directories(Marble.Mathematics PUBLIC
        "${CMAKE_SOURCE_DIR}/Marble.Mathematics/src"
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
    )
elseif (${BUILD_PLATFORM} STREQUAL Linux)
    target_link_libraries(Marble.Mathematics PUBLIC
    )
    target_include_directories(Marble.Mathematics PUBLIC
        "${CMAKE_SOURCE_DIR}/Marble.Mathematics/src"
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
    )
endif()

# Marble.Runtime.GL

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Runtime.GL)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Runtime.GL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Runtime.GL)

file(GLOB_RECURSE GLSRCS configure_depends
    "${CMAKE_SOURCE_DIR}/Marble.Runtime.GL/*.h"
    "${CMAKE_SOURCE_DIR}/Marble.Runtime.GL/*.cpp"
)

add_library(Marble.Runtime.GL SHARED ${GLSRCS})
target_compile_definitions(Marble.Runtime.GL PUBLIC BUILD_TYPE_DYNAMIC=1)

if (${BUILD_PLATFORM} STREQUAL Win32)
    target_include_directories(Marble.Runtime.GL PUBLIC
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.GL/src"
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
        "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bgfx/include"
        "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bx/include"
        "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bimg/include"
    )
    target_link_libraries(Marble.Runtime.GL PUBLIC
        gl
    )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_include_directories(Marble.Runtime.GL PUBLIC
            "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bx/include/compat/mingw"
        )
    else()
    endif()
elseif (${BUILD_PLATFORM} STREQUAL Linux)
    target_link_libraries(Marble.Runtime.GL PUBLIC
    )
    target_include_directories(Marble.Runtime.GL PUBLIC
    )
endif()

# Marble.Runtime.CoreLib

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Runtime.CoreLib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Runtime.CoreLib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Runtime.CoreLib)

file(GLOB_RECURSE ENGINESRCS configure_depends
    "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/*.h"
    "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/*.cpp"
)

add_library(Marble.Runtime.CoreLib SHARED ${ENGINESRCS})
target_compile_definitions(Marble.Runtime.CoreLib PUBLIC SDL_MAIN_HANDLED BUILD_TYPE_DYNAMIC=1)

target_include_directories(Marble.Runtime.CoreLib PUBLIC
    "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
    "${CMAKE_SOURCE_DIR}/Marble.Runtime.GL/src"
    "${CMAKE_SOURCE_DIR}/Marble.Mathematics/src"
    "${CMAKE_SOURCE_DIR}/dependencies/ctti/include"
    "${CMAKE_SOURCE_DIR}/dependencies/stb"
    "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bgfx/include"
    "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bx/include"
    "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bimg/include"
    "${CMAKE_SOURCE_DIR}/external/SDL2/include"
)
target_link_libraries(Marble.Runtime.CoreLib PUBLIC
    Marble.Mathematics
    Marble.Runtime.GL
    SDL2-static
    SDL2main
)
if (${BUILD_PLATFORM} STREQUAL Win32)
    target_include_directories(Marble.Runtime.CoreLib PUBLIC
    )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_include_directories(Marble.Runtime.CoreLib PUBLIC
            "${CMAKE_SOURCE_DIR}/dependencies/bgfx/include/compat/mingw"
        )
    else()
    endif()
    target_link_libraries(Marble.Runtime.CoreLib PUBLIC
        psapi
    )
elseif (${BUILD_PLATFORM} STREQUAL Linux)
    target_link_libraries(Marble.Runtime.CoreLib PUBLIC
        ${CMAKE_DL_LIBS}
    )
    target_include_directories(Marble.Runtime.CoreLib PUBLIC
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
    )
endif()

# Sandbox

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Sandbox)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Sandbox)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Sandbox)

file(GLOB_RECURSE SANDBOXSRCS configure_depends
    "${CMAKE_SOURCE_DIR}/Sandbox/src/*.h"
    "${CMAKE_SOURCE_DIR}/Sandbox/src/*.cpp"
)

add_executable(Sandbox ${SANDBOXSRCS})
target_compile_definitions(Sandbox PUBLIC SDL_MAIN_HANDLED)
message(STATUS ${CMAKE_EXE_LINKER_FLAGS})

if (${BUILD_PLATFORM} STREQUAL Win32)
    target_link_libraries(Sandbox PUBLIC
        Marble.Mathematics
        Marble.Runtime.CoreLib
    )
    target_include_directories(Sandbox PUBLIC
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/include"
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
        "${CMAKE_SOURCE_DIR}/Marble.Mathematics/src"
        "${CMAKE_SOURCE_DIR}/dependencies/ctti/include"
        "${CMAKE_SOURCE_DIR}/external/SDL/include"
    )

    add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/bin/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-Win32/Marble.Mathematics/Marble.Mathematics.dll"
        $<TARGET_FILE_DIR:Sandbox>
    )
    add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/bin/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-Win32/Marble.Runtime.GL/Marble.Runtime.GL.dll"
        $<TARGET_FILE_DIR:Sandbox>
    )
    add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/bin/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-Win32/Marble.Runtime.CoreLib/Marble.Runtime.CoreLib.dll"
        $<TARGET_FILE_DIR:Sandbox>
    )
elseif (${BUILD_PLATFORM} STREQUAL Linux)
    target_link_libraries(Sandbox PUBLIC
    )
    target_include_directories(Sandbox PUBLIC
    )
endif()
