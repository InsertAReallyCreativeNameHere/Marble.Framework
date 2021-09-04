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

ShaderCompileOptions::ShaderCompileOptions(ShaderType type)
{
    this->shaderType = type;
}

ShaderCompileOptions& ShaderCompileOptions::withIncludeDirs(const std::vector<std::string>& includeDirs)
{
    this->includeDirs.insert(this->includeDirs.end(), includeDirs.begin(), includeDirs.end());

    return *this;
}
ShaderCompileOptions& ShaderCompileOptions::withDefines(const std::vector<std::string>& defines)
{
    this->defines.insert(this->defines.end(), defines.begin(), defines.end());

    return *this;
}

std::vector<char> ShaderUtility::compileShader(const std::string& shaderData, const ShaderCompileOptions& options)
{
    char shaderType[2] { 0 };
    shaderType[0] = (char)options.shaderType;

    std::string varyingPath(curPath);
    varyingPath.append("/Runtime/varying.def.sc");

    std::string prof;
    prof.reserve(6);
    prof.push_back((char)options.shaderType);
    prof.push_back("s_3_0");
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
    args.push_back(prof.c_str());

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
