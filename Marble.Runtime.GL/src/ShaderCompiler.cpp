#include "ShaderCompiler.h"

#include <fstream>
#include <shaderc.h>

using namespace Marble;

ShaderCompileOptions::ShaderCompileOptions(ShaderType type)
{
    switch (type)
    {
    case ShaderType::Vertex:
        this->shaderType = "vertex";
        break;
    case ShaderType::Fragment:
        this->shaderType = "fragment";
        break;
    case ShaderType::Compute:
        this->shaderType = "compute";
        break;
    }
}

ShaderCompileOptions& ShaderCompileOptions::withIncludeDirs(const std::vector<std::string>& includeDirs)
{
    this->includeDirs.reserve(includeDirs.size() * 2);
    for (auto it = includeDirs.begin(); it != includeDirs.end(); ++it)
    {
        this->includeDirs.push_back("-i");
        std::string includePath;
        includePath.reserve(it->size());
        includePath.append("\"");
        includePath.append(*it);
        includePath.append("\"");
        this->includeDirs.push_back(std::move(includePath));
    }

    return *this;
}
ShaderCompileOptions& ShaderCompileOptions::withDefines(const std::vector<std::string>& defines)
{
    for (auto it = defines.begin(); it != defines.end() - 1; ++it)
    {
        this->defines.append(*it);
        this->defines.append(";");
    }
    this->defines.append(defines.back());

    return *this;
}

const bgfx::Memory* ShaderCompiler::compileShader(const std::string_view& fileName, const ShaderCompileOptions& options)
{
    std::string outputPath;
    outputPath.reserve(fileName.size() + 1);
    outputPath.append(fileName);
    *(outputPath.end() - 3) = 'b';
    *(outputPath.end() - 2) = 'i';
    *(outputPath.end() - 1) = 'n';

    std::vector<const char*> args({ "-f", fileName.begin(), "-o", outputPath.c_str(), "--varyingdef", "Runtime/varying.def.sc", "--verbose", "--type" });
    args.push_back(options.shaderType.c_str());

    for (auto it = options.includeDirs.begin(); it != options.includeDirs.end(); ++it)
        args.push_back(it->c_str());

    if (options.defines != "")
    {
        args.push_back("--define");
        args.push_back(options.defines.c_str());
    }

    bgfx::compileShader(1, args.data());

    std::ifstream shaderFile(outputPath, std::ios::binary);
    shaderFile.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize length = shaderFile.gcount();
    shaderFile.clear();
    shaderFile.seekg(0, std::ios::beg);
    
    char* shaderBytes = new char[length];
    shaderFile.read(shaderBytes, length);
    return bgfx::makeRef(shaderBytes, length);
}