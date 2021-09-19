cmake_minimum_required(VERSION 3.0)

project(gl)

set(CMAKE_CXX_STANDARD 14)
if (COMPILER STREQUAL "MinGW")
    add_definitions(-DMINGW_HAS_SECURE_API=0)
endif()

set(BGFX_LIBRARY_TYPE "STATIC")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../bgfx.cmake" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
include("${CMAKE_CURRENT_LIST_DIR}/../bgfx.cmake/cmake/3rdparty/glslang.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../bgfx.cmake/cmake/3rdparty/glsl-optimizer.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../bgfx.cmake/cmake/3rdparty/fcpp.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../bgfx.cmake/cmake/3rdparty/spirv-tools.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../bgfx.cmake/cmake/3rdparty/spirv-cross.cmake")

add_library(gl "src.cpp")
target_include_directories(gl PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/../bgfx.cmake/bgfx/3rdparty/fcpp"
    "${CMAKE_CURRENT_LIST_DIR}/../bgfx.cmake/bgfx/3rdparty/glsl-optimizer/src/glsl"
    "${CMAKE_CURRENT_LIST_DIR}/../bgfx.cmake/bgfx/3rdparty/glslang/glslang/Public"
    "${CMAKE_CURRENT_LIST_DIR}/../bgfx.cmake/bgfx/3rdparty/glslang/glslang/Include"
    "${CMAKE_CURRENT_LIST_DIR}/../bgfx.cmake/bgfx/3rdparty/glslang"
    "${CMAKE_CURRENT_LIST_DIR}/../bgfx.cmake/bgfx/3rdparty/spirv-tools/include"
    "${CMAKE_CURRENT_LIST_DIR}/../bgfx.cmake/bgfx/3rdparty/spirv-cross"
    "${CMAKE_CURRENT_LIST_DIR}/../bgfx.cmake/bgfx/3rdparty/webgpu/include"
    "${CMAKE_CURRENT_LIST_DIR}/../libglcompilers"
)
target_link_libraries(gl PRIVATE
    bgfx
    bx
    bimg

    glslang
    glsl-optimizer
    fcpp
    spirv-tools
    spirv-cross
)