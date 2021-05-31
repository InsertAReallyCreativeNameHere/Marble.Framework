#pragma once

#include <inc.h>

#include <bgfx/bgfx.h>
#include <string_view>

namespace Marble
{
    enum class ShaderType : uint8_t
    {
        Vertex,
        Fragment,
        Compute
    };

    class ShaderCompiler;

    struct coreapi ShaderCompileOptions final
    {
        ShaderCompileOptions(ShaderType type);

        ShaderCompileOptions& withIncludeDirs(const std::vector<std::string>& includeDirs);
        ShaderCompileOptions& withDefines(const std::vector<std::string>& defines);

        friend class Marble::ShaderCompiler;
    private:
        std::vector<std::string> includeDirs;
        std::string defines;
        std::string shaderType;
    };

    class coreapi ShaderCompiler final
    {
    public:
        static const bgfx::Memory* compileShader(const std::string_view& fileName, const ShaderCompileOptions& options);
    };
}