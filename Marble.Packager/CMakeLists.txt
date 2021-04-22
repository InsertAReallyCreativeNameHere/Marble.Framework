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

project(Marble.Packager)

set(CMAKE_SHARED_LIBRARY_PREFIX "")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++20 -fopenmp -fconcepts")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++20 -pthread")
else()
    message(FATAL_ERROR "No supported compiler used.")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Packager)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib/${BUILD_ARCH}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Packager)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin/${BUILD_ARCH}-${CMAKE_BUILD_TYPE}-${BUILD_PLATFORM}/Marble.Packager)

file(GLOB_RECURSE PKGSRCS configure_depends
    "${CMAKE_SOURCE_DIR}/src/*.h"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(Marble.Packager ${PKGSRCS})
target_compile_definitions(Marble.Packager PUBLIC BUILD_TYPE_DYNAMIC=1)

if (${BUILD_PLATFORM} STREQUAL Win32)
    target_link_libraries(Marble.Packager PUBLIC
        "${CMAKE_SOURCE_DIR}/../vendor/zip/lib/libzip.a"
    )
    target_include_directories(Marble.Packager PUBLIC
        "${CMAKE_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/../vendor/zip/include"
    )
elseif (${BUILD_PLATFORM} STREQUAL Linux)
    target_link_libraries(Marble.Packager PUBLIC
        "${CMAKE_SOURCE_DIR}/../vendor/zip/lib/libzip.a"
    )
    target_include_directories(Marble.Packager PUBLIC
        "${CMAKE_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/../vendor/zip/include"
    )
endif()