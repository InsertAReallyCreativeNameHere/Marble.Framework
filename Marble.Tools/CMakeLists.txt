cmake_minimum_required(VERSION 3.10)

project(Marble.Tools)

add_executable(Marble.Packager
    "${CMAKE_CURRENT_LIST_DIR}/src/packager_main.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/Packager.cpp"
)
target_include_directories(Marble.Packager PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")

set_target_properties(Marble.Packager
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../../lib/${OUTNAME}/Marble.Packager"
    LIBRARY_OUTPUT_DIRECTORY "../../lib/${OUTNAME}/Marble.Packager"
    RUNTIME_OUTPUT_DIRECTORY "../../bin/${OUTNAME}/Marble.Packager"
)

add_executable(Marble.ShaderCompiler
    "${CMAKE_CURRENT_LIST_DIR}/src/packager.sc_main.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/Packager.cpp"
)
target_include_directories(Marble.ShaderCompiler PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")
target_compile_definitions(Marble.ShaderCompiler PRIVATE CMAKE_EXECUTABLE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX})

set_target_properties(Marble.ShaderCompiler
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "../../lib/${OUTNAME}/Marble.ShaderCompiler"
    LIBRARY_OUTPUT_DIRECTORY "../../lib/${OUTNAME}/Marble.ShaderCompiler"
    RUNTIME_OUTPUT_DIRECTORY "../../bin/${OUTNAME}/Marble.ShaderCompiler"
)
add_custom_command(TARGET Marble.ShaderCompiler POST_BUILD
    COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:shaderc>
    $<TARGET_FILE_DIR:Marble.ShaderCompiler>
)
