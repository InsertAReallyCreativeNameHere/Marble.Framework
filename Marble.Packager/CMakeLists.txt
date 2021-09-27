cmake_minimum_required(VERSION 3.10)

project(Marble.Packager)

file(GLOB_RECURSE PKGSRCS configure_depends
    "${CMAKE_CURRENT_LIST_DIR}/src/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)
add_executable(Marble.Packager ${PKGSRCS})
target_compile_definitions(Marble.Packager PRIVATE BUILD_TYPE_DYNAMIC=1)
target_include_directories(Marble.Packager PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")

set_target_properties(Marble.Packager
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../../lib/${OUTNAME}/Marble.Packager"
    LIBRARY_OUTPUT_DIRECTORY "../../lib/${OUTNAME}/Marble.Packager"
    RUNTIME_OUTPUT_DIRECTORY "../../bin/${OUTNAME}/Marble.Packager"
)
