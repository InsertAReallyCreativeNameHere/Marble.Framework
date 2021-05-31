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

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Init)

set(CMAKE_SHARED_LIBRARY_PREFIX "")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fconcepts -Wall -Wextra -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-but-set-variable")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    endif()
    if (CMAKE_SHARED_LIBRARY_SUFFIX STREQUAL ".dll")
        set(COMPILER "MinGW")
    else()
        set(COMPILER "gcc")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=lld -std=c++20w -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-unknown-pragmas -Wno-sign-compare -Wno-reserved-id-macro")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-comma -Wno-undef -Wno-source-uses-openmp")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-global-constructors -Wno-exit-time-destructors -Wno-float-equal -Wno-shadow-field-in-constructor")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-newline-eof -Wno-implicit-fallthrough -Wno-padded -Wno-sign-conversion")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-old-style-cast -Wno-mismatched-tags -Wno-shadow -Wno-double-promotion -Wno-reorder-ctor")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-weak-vtables -Wno-unused-template -Wno-zero-as-null-pointer-constant -Wno-cast-qual")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-disabled-macro-expansion -Wno-cast-align -Wno-implicit-int-conversion -Wno-overloaded-virtual")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-extra-semi-stmt -Wno-implicit-int-float-conversion -Wno-used-but-marked-unused")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-suggest-destructor-override -Wno-shadow-field -Wno-shorten-64-to-32 -Wno-unused-variable")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    endif()
    set(COMPILER "Clang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
else()
    message(FATAL_ERROR "No supported compiler used.")
endif()

set(SDL_SHARED OFF)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/SDL")

add_subdirectory("${CMAKE_SOURCE_DIR}/external/bgfx.cmake")
include("${CMAKE_SOURCE_DIR}/external/bgfx.cmake/cmake/3rdparty/glslang.cmake")
include("${CMAKE_SOURCE_DIR}/external/bgfx.cmake/cmake/3rdparty/glsl-optimizer.cmake")
include("${CMAKE_SOURCE_DIR}/external/bgfx.cmake/cmake/3rdparty/fcpp.cmake")
include("${CMAKE_SOURCE_DIR}/external/bgfx.cmake/cmake/3rdparty/spirv-tools.cmake")
include("${CMAKE_SOURCE_DIR}/external/bgfx.cmake/cmake/3rdparty/spirv-cross.cmake")

project(Marble.Mathematics)

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

project(Marble.Runtime.GL)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Runtime.GL)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Runtime.GL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Runtime.GL)

file(GLOB_RECURSE GLSRCS configure_depends
    "${CMAKE_SOURCE_DIR}/Marble.Runtime.GL/*.h"
    "${CMAKE_SOURCE_DIR}/Marble.Runtime.GL/*.cpp"
)
file(GLOB_RECURSE SHADERC configure_depends
    "${CMAKE_SOURCE_DIR}/external/shaderc_modified/*.h"
    "${CMAKE_SOURCE_DIR}/external/shaderc_modified/*.cpp"
)

add_library(Marble.Runtime.GL SHARED ${GLSRCS} ${SHADERC})
target_compile_definitions(Marble.Runtime.GL PUBLIC BUILD_TYPE_DYNAMIC=1)

if (${BUILD_PLATFORM} STREQUAL Win32)
    target_include_directories(Marble.Runtime.GL PUBLIC
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.GL/src"
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
        "${CMAKE_SOURCE_DIR}/dependencies/bgfx/include"
        "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bgfx/3rdparty/fcpp"
        "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bgfx/3rdparty/glsl-optimizer/src/glsl"
        "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bgfx/3rdparty/glslang/glslang/Public"
        "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bgfx/3rdparty/glslang/glslang/Include"
        "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bgfx/3rdparty/glslang"
        "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bgfx/3rdparty/spirv-tools/include"
        "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bgfx/3rdparty/spirv-cross"
        "${CMAKE_SOURCE_DIR}/external/bgfx.cmake/bgfx/3rdparty/webgpu/include"
        "${CMAKE_SOURCE_DIR}/external/shaderc_modified"
    )
    target_link_libraries(Marble.Runtime.GL PUBLIC
        bgfx
        bx
        bimg
        fcpp
        glsl-optimizer
        glslang
        spirv-tools
        spirv-cross
        psapi
    )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_include_directories(Marble.Runtime.GL PUBLIC
            "${CMAKE_SOURCE_DIR}/dependencies/bgfx/include/compat/mingw"
        )
    else()
    endif()
elseif (${BUILD_PLATFORM} STREQUAL Linux)
    target_link_libraries(Marble.Runtime.GL PUBLIC
    )
    target_include_directories(Marble.Runtime.GL PUBLIC
    )
endif()

project(Marble.Runtime.CoreLib)

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
    "${CMAKE_SOURCE_DIR}/dependencies/bgfx/include"
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

project(Sandbox)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Sandbox)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Sandbox)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Sandbox)

file(GLOB_RECURSE SANDBOXSRCS configure_depends
    "${CMAKE_SOURCE_DIR}/Sandbox/src/*.h"
    "${CMAKE_SOURCE_DIR}/Sandbox/src/*.cpp"
)

add_executable(Sandbox ${SANDBOXSRCS})
add_dependencies(Sandbox Marble.Runtime.CoreLib)
target_compile_definitions(Sandbox PUBLIC SDL_MAIN_HANDLED)

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
