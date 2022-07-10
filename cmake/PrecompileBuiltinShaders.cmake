include(${CMAKE_CURRENT_LIST_DIR}/cmake-bin2h/bin2h.cmake)

message(STATUS "[Shader Precompile] Compiling shaders...")

file(MAKE_DIRECTORY "${SHADER_FILES_DIR}/build")

# 2DPolygon Vertex
execute_process(
    COMMAND "${SHADERC_PATH}" "${SHADER_FILES_DIR}/configure/2DPolygon.vert.sc" "${SHADER_FILES_DIR}/build/2DPolygon.vert.mpsh" v
    --varyingdef "${SHADER_FILES_DIR}/configure/2DPolygon.varying.def.sc" -i "${BGFX_SHADER_DIR}"
    WORKING_DIRECTORY "${SHADERC_DIR}"
    RESULT_VARIABLE SHADERC_RETURN
)
if (SHADERC_RETURN AND NOT SHADERC_RETURN EQUAL 0)
    message(FATAL_ERROR "[Shader Precompile] Shader compilation of vertex shader 2DPolygon failed with code [${SHADERC_RETURN}].")
else()
    message(STATUS "[Shader Precompile] Shader compilation of vertex shader 2DPolygon succeeded.")
endif()

# 2DPolygon Fragment
execute_process(
    COMMAND "${SHADERC_PATH}" "${SHADER_FILES_DIR}/configure/2DPolygon.frag.sc" "${SHADER_FILES_DIR}/build/2DPolygon.frag.mpsh" f
    --varyingdef "${SHADER_FILES_DIR}/configure/2DPolygon.varying.def.sc" -i "${BGFX_SHADER_DIR}"
    WORKING_DIRECTORY "${SHADERC_DIR}"
    RESULT_VARIABLE SHADERC_RETURN
)
if (SHADERC_RETURN AND NOT SHADERC_RETURN EQUAL 0)
    message(FATAL_ERROR "[Shader Precompile] Shader compilation of fragment shader 2DPolygon failed with code [${SHADERC_RETURN}].")
else()
    message(STATUS "[Shader Precompile] Shader compilation of fragment shader 2DPolygon succeeded.")
endif()

# Textured2DPolygon Vertex
execute_process(
    COMMAND "${SHADERC_PATH}" "${SHADER_FILES_DIR}/configure/Textured2DPolygon.vert.sc" "${SHADER_FILES_DIR}/build/Textured2DPolygon.vert.mpsh" v
    --varyingdef "${SHADER_FILES_DIR}/configure/Textured2DPolygon.varying.def.sc" -i "${BGFX_SHADER_DIR}"
    WORKING_DIRECTORY "${SHADERC_DIR}"
    RESULT_VARIABLE SHADERC_RETURN
)
if (SHADERC_RETURN AND NOT SHADERC_RETURN EQUAL 0)
    message(FATAL_ERROR "[Shader Precompile] Shader compilation of vertex shader Textured2DPolygon failed with code [${SHADERC_RETURN}].")
else()
    message(STATUS "[Shader Precompile] Shader compilation of vertex shader Textured2DPolygon succeeded.")
endif()

# Textured2DPolygon Fragment
execute_process(
    COMMAND "${SHADERC_PATH}" "${SHADER_FILES_DIR}/configure/Textured2DPolygon.frag.sc" "${SHADER_FILES_DIR}/build/Textured2DPolygon.frag.mpsh" f
    --varyingdef "${SHADER_FILES_DIR}/configure/Textured2DPolygon.varying.def.sc" -i "${BGFX_SHADER_DIR}"
    WORKING_DIRECTORY "${SHADERC_DIR}"
    RESULT_VARIABLE SHADERC_RETURN
)
if (SHADERC_RETURN AND NOT SHADERC_RETURN EQUAL 0)
    message(FATAL_ERROR "[Shader Precompile] Shader compilation of fragment shader Textured2DPolygon failed with code [${SHADERC_RETURN}].")
else()
    message(STATUS "[Shader Precompile] Shader compilation of fragment shader Textured2DPolygon succeeded.")
endif()

bin2h(
    SOURCE_FILE "${SHADER_FILES_DIR}/build/2DPolygon.vert.mpsh"
    HEADER_FILE "${SHADER_FILES_DIR}/2DPolygon.vert.h"
    VARIABLE_NAME "shader2DPolygonVertexData"
)
bin2h(
    SOURCE_FILE "${SHADER_FILES_DIR}/build/2DPolygon.frag.mpsh"
    HEADER_FILE "${SHADER_FILES_DIR}/2DPolygon.frag.h"
    VARIABLE_NAME "shader2DPolygonFragmentData"
)
bin2h(
    SOURCE_FILE "${SHADER_FILES_DIR}/build/Textured2DPolygon.vert.mpsh"
    HEADER_FILE "${SHADER_FILES_DIR}/Textured2DPolygon.vert.h"
    VARIABLE_NAME "shaderTextured2DPolygonVertexData"
)
bin2h(
    SOURCE_FILE "${SHADER_FILES_DIR}/build/Textured2DPolygon.frag.mpsh"
    HEADER_FILE "${SHADER_FILES_DIR}/Textured2DPolygon.frag.h"
    VARIABLE_NAME "shaderTextured2DPolygonFragmentData"
)

message(STATUS "[Shader Precompile] Shader precompilation complete.")