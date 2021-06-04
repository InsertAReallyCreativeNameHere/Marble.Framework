cmake_minimum_required( VERSION 3.0 )

set(_CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})
set(CMAKE_BUILD_TYPE "Release")
set(BGFX_LIBRARY_TYPE "STATIC")
add_subdirectory("${CMAKE_SOURCE_DIR}/external/bgfx.cmake" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
include("${CMAKE_SOURCE_DIR}/external/bgfx.cmake/cmake/3rdparty/glslang.cmake")
include("${CMAKE_SOURCE_DIR}/external/bgfx.cmake/cmake/3rdparty/glsl-optimizer.cmake")
include("${CMAKE_SOURCE_DIR}/external/bgfx.cmake/cmake/3rdparty/fcpp.cmake")
include("${CMAKE_SOURCE_DIR}/external/bgfx.cmake/cmake/3rdparty/spirv-tools.cmake")
include("${CMAKE_SOURCE_DIR}/external/bgfx.cmake/cmake/3rdparty/spirv-cross.cmake")
set(CMAKE_BUILD_TYPE ${_CMAKE_BUILD_TYPE})

project(gl)
add_library(gl "src.cpp")
target_include_directories(gl PUBLIC
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
target_link_libraries(gl PUBLIC
    bgfx
    bx
    bimg

    glslang
    glsl-optimizer
    fcpp
    spirv-tools
    spirv-cross
)