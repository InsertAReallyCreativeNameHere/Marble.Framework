cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE TESSSRCS configure_depends "${CMAKE_SOURCE_DIR}/external/libtess2/Source/*.c")
add_library(tess STATIC ${TESSSRCS})
target_include_directories(tess PRIVATE "${CMAKE_SOURCE_DIR}/external/libtess2/Source" "${CMAKE_SOURCE_DIR}/external/libtess2/Include")