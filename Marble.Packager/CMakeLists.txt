cmake_minimum_required(VERSION 3.10)

project(Marble.Packager)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../../lib/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Marble.Packager)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../lib/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Marble.Packager)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../bin/${BUILD_ARCH}-${COMPILER}-${BUILD_TYPE}${BUILD_PLATFORM}/Marble.Packager)

file(GLOB_RECURSE PKGSRCS configure_depends
    "${CMAKE_SOURCE_DIR}/Marble.Packager/src/*.h"
    "${CMAKE_SOURCE_DIR}/Marble.Packager/src/*.cpp"
)
add_executable(Marble.Packager ${PKGSRCS})
target_compile_definitions(Marble.Packager PRIVATE BUILD_TYPE_DYNAMIC=1)
target_include_directories(Marble.Packager PRIVATE "${CMAKE_SOURCE_DIR}/Marble.Packager/src")