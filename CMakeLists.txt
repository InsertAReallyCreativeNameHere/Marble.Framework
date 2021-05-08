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

project(Marble.Mathematics)

set(CMAKE_SHARED_LIBRARY_PREFIX "")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fconcepts -flto -Wall -Wextra -pedantic-errors -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-but-set-variable")
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=lld -std=c++20 -flto -pthread")
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
else()
    message(FATAL_ERROR "No supported compiler used.")
endif()

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

if (${BUILD_PLATFORM} STREQUAL Win32)
    target_include_directories(Marble.Runtime.CoreLib PUBLIC
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
        "${CMAKE_SOURCE_DIR}/Marble.Mathematics/src"
        "${CMAKE_SOURCE_DIR}/dependencies/ctti/include"
        "${CMAKE_SOURCE_DIR}/dependencies/stb"
        "${CMAKE_SOURCE_DIR}/dependencies/bgfx/include"
    )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_include_directories(Marble.Runtime.CoreLib PUBLIC "${CMAKE_SOURCE_DIR}/dependencies/SDL2-MinGW/include/SDL2")
    else()
        target_include_directories(Marble.Runtime.CoreLib PUBLIC "${CMAKE_SOURCE_DIR}/dependencies/SDL2/include")
    endif()
    target_link_libraries(Marble.Runtime.CoreLib PUBLIC
        Marble.Mathematics
        "${CMAKE_SOURCE_DIR}/dependencies/bgfx/${BUILD_ARCH}-mingw-win/lib/libbxRelease.a"
        "${CMAKE_SOURCE_DIR}/dependencies/bgfx/${BUILD_ARCH}-mingw-win/lib/libbimgRelease.a"
        "${CMAKE_SOURCE_DIR}/dependencies/bgfx/${BUILD_ARCH}-mingw-win/lib/libbgfxRelease.a"
    )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(Marble.Runtime.CoreLib PUBLIC 
            "${CMAKE_SOURCE_DIR}/dependencies/SDL2-MinGW/lib/libSDL2main.a"
            "${CMAKE_SOURCE_DIR}/dependencies/SDL2-MinGW/lib/libSDL2.dll.a"
        )
    else()
        target_link_libraries(Marble.Runtime.CoreLib PUBLIC 
            "${CMAKE_SOURCE_DIR}/dependencies/SDL2/lib/x86/SDL2main.lib"
            "${CMAKE_SOURCE_DIR}/dependencies/SDL2/lib/x86/SDL2.lib"
        )
    endif()
elseif (${BUILD_PLATFORM} STREQUAL Linux)
    target_link_libraries(Marble.Runtime.CoreLib PUBLIC
        "${CMAKE_SOURCE_DIR}/dependencies/SDL2-Linux/lib/libSDL2.a"
        "${CMAKE_SOURCE_DIR}/dependencies/SDL2-Linux/lib/libSDL2main.a"
        "${CMAKE_SOURCE_DIR}/lib/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/ImGui/libImGui.a"
        "${CMAKE_SOURCE_DIR}/lib/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/gl3w/libgl3w.a"
        ${CMAKE_DL_LIBS}
    )
    target_include_directories(Marble.Runtime.CoreLib PUBLIC
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
        "${CMAKE_SOURCE_DIR}/gl3w/include"
        "${CMAKE_SOURCE_DIR}/ImGUI"
        "${CMAKE_SOURCE_DIR}/dependencies/SDL2-Linux/include/SDL2"
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
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src/include"
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
        "${CMAKE_SOURCE_DIR}/Marble.Mathematics/src"
        "${CMAKE_SOURCE_DIR}/dependencies/ctti/include"
    )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_include_directories(Sandbox PUBLIC "${CMAKE_SOURCE_DIR}/dependencies/SDL2-MinGW/include/SDL2")
    else()
        target_include_directories(Sandbox PUBLIC "${CMAKE_SOURCE_DIR}/dependencies/SDL2/include")
    endif()
    add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/bin/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-Win32/Marble.Mathematics/Marble.Mathematics.dll"
        $<TARGET_FILE_DIR:Sandbox>
    )
    add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/bin/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-Win32/Marble.Runtime.CoreLib/Marble.Runtime.CoreLib.dll"
        $<TARGET_FILE_DIR:Sandbox>
    )
    add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/runtime/C++"
        $<TARGET_FILE_DIR:Sandbox>
    )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_custom_command(TARGET Sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/dependencies/SDL2-MinGW/bin/SDL2.dll"
            $<TARGET_FILE_DIR:Sandbox>
        )
    else()
        add_custom_command(TARGET Sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/dependencies/SDL2/lib/${BUILD_ARCH}/SDL2.dll"
            $<TARGET_FILE_DIR:Sandbox>
        )
    endif()
elseif (${BUILD_PLATFORM} STREQUAL Linux)
    target_link_libraries(Sandbox PUBLIC
        "${CMAKE_SOURCE_DIR}/lib/${BUILD_ARCH}-${COMPILER}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Runtime.CoreLib/Marble.Runtime.CoreLib.so"
    )
    target_include_directories(Sandbox PUBLIC
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src/include"
        "${CMAKE_SOURCE_DIR}/Marble.Runtime.CoreLib/src"
        "${CMAKE_SOURCE_DIR}/GLAD/include"
        "${CMAKE_SOURCE_DIR}/ImGUI"
        "${CMAKE_SOURCE_DIR}/dependencies/SDL2-Linux/include/SDL2"
    )
endif()
