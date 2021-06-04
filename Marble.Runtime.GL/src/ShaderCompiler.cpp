#include "ShaderCompiler.h"

#include <filesystem>
#include <fstream>
#include <shaderc.h>

using namespace Marble;
using namespace Marble::Internal;
namespace fs = std::filesystem;

static std::string curPath(fs::current_path().string());

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

std::vector<uint8_t> ShaderCompiler::callInternalMain(int argc, const char* argv[])
{
    return bgfx::compileShader(argc, argv);
}

ShaderCompiler::Initializer::Initializer()
{
    bgfx::g_verbose = true;
}

std::vector<uint8_t> ShaderCompiler::compileShader(const std::string_view& fileName, const ShaderCompileOptions& options)
{
    std::string inputFilePath;
    inputFilePath.reserve(curPath.size() + fileName.size() + 1);
    inputFilePath.append(curPath);
    inputFilePath.append("/");
    inputFilePath.append(fileName);

    std::string outputFilePath(inputFilePath);
    outputFilePath.append(".bin");

    char shaderType[2] { 0 };
    shaderType[0] = (char)options.shaderType;

    std::string varyingPath(curPath);
    varyingPath.append("/Runtime/varying.def.sc");

    std::vector<const char*> args
    {
        "-f", inputFilePath.c_str(),
        "-o", outputFilePath.c_str(),
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
        "-i", fs::path(fileName.data()).parent_path().string().c_str(),
        "--varyingdef", varyingPath.c_str(),
        "-p"
    };
    args.push_back("spirv");

    for (auto it = options.includeDirs.begin(); it != options.includeDirs.end(); ++it)
    {
        puts(it->c_str());
        args.push_back("-i");
        args.push_back(it->c_str());
    }
    
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

    std::ifstream shaderFile(inputFilePath, std::ios::binary);
    shaderFile.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize length = shaderFile.gcount();
    shaderFile.clear();
    shaderFile.seekg(0, std::ios::beg);
    char* shaderBytes = new char[length];
    shaderFile.read(shaderBytes, length);
    
    return bgfx::compileShader(args.size(), args.data());

    // writer = new bx::MemoryWriter(new bx::MemoryBlock(new bx::DefaultAllocator));
}