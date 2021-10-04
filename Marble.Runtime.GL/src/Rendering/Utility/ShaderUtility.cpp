#include "ShaderUtility.h"

#include <filesystem>
#include <fstream>
#include <shaderc.h>

using namespace Marble;
using namespace Marble::GL;
namespace fs = std::filesystem;

static std::string curPath(fs::current_path().string());

static const char* varyingdefDefault =
R"(
vec4 v_color0    : COLOR0    = vec4(1.0, 0.0, 0.0, 1.0);
vec4 v_color1    : COLOR1    = vec4(0.0, 1.0, 0.0, 1.0);
vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);
vec3 v_normal    : TEXCOORD1 = vec3(0.0, 1.0, 0.0);

vec3 a_position  : POSITION;
vec3 a_normal    : NORMAL0;
vec4 a_color0    : COLOR0;
vec4 a_color1    : COLOR1;
vec2 a_texcoord0 : TEXCOORD0;
)";

std::vector<char> ShaderUtility::compileShader(const std::string& shaderData, const ShaderCompileOptions& options)
{
    ProfileFunction();
    
    char shaderType[2] { 0 };
    shaderType[0] = (char)options.shaderType;

    std::string varyingPath(curPath);
    varyingPath.append("/Runtime/varying.def.sc");

    std::vector<const char*> args
    {
        #if BX_PLATFORM_LINUX
        "--platform", "linux",
        #elif BX_PLATFORM_WINDOWS
        "--platform", "windows",
        #elif BX_PLATFORM_ANDROID
        "--platform", "android",
        #elif BX_PLATFORM_EMSCRIPTEN
        "--platform", "asm.js",
        #elif BX_PLATFORM_IOS
        "--platform", "ios",
        #elif BX_PLATFORM_OSX
        "--platform", "osx",
        #endif
        "--type", shaderType,
        "-p"
    };

    std::string profile =
    [&]
    {
        switch(bgfx::getRendererType())
        {
        case bgfx::RendererType::Direct3D9:
            switch (options.shaderType)
            {
            case ShaderType::Vertex:
                return "vs_3_0";
            case ShaderType::Fragment:
            case ShaderType::Compute:
                return "ps_3_0";
            }
        case bgfx::RendererType::Direct3D11:
            switch (options.shaderType)
            {
            case ShaderType::Vertex:
                return "vs_4_0";
            case ShaderType::Fragment:
                return "ps_4_0";
            case ShaderType::Compute:
                return "cs_5_0";
            }
        case bgfx::RendererType::Direct3D12:
            switch (options.shaderType)
            {
            case ShaderType::Vertex:
                return "vs_5_0";
            case ShaderType::Fragment:
                return "ps_5_0";
            case ShaderType::Compute:
                return "cs_5_0";
            }
        case bgfx::RendererType::OpenGL:
            switch (options.shaderType)
            {
            case ShaderType::Vertex:
            case ShaderType::Fragment:
                return "120";
            case ShaderType::Compute:
                return "430";
            }
        case bgfx::RendererType::Vulkan:
            return "spirv";
        case bgfx::RendererType::Gnm:
        case bgfx::RendererType::Metal:
        case bgfx::RendererType::OpenGLES:
        case bgfx::RendererType::Noop:
        default:
            return "unknown";
        };
    }
    ();
    args.push_back(profile.c_str());

    for (auto it = options.includeDirs.begin(); it != options.includeDirs.end(); ++it)
    {
        puts(it->c_str());
        args.push_back("-i");
        args.push_back(it->c_str());
    }
    args.push_back("-i");
    args.push_back(curPath.c_str());
    
    if (options.defines.size() > 0)
    {
        args.push_back("--define");
        std::string defines;
        for (auto it = options.defines.begin(); it != options.defines.end() - 1; ++it)
        {
            defines.append(*it);
            defines.append(";");
        }
        defines.append(*(options.defines.end() - 1));
        args.push_back(defines.c_str());
    }

    return bgfx::compileShader(shaderData.data(), shaderData.size(), varyingdefDefault, args);
}
