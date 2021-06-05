#pragma once

#include "inc.h"

#include <filesystem>

namespace Marble
{
    namespace GL
    {
        enum class ShaderType : char
        {
            Vertex = 'v',
            Fragment = 'f',
            Compute = 'c'
        };

        class ShaderCompiler;

        struct coreapi ShaderCompileOptions final
        {
            ShaderCompileOptions(ShaderType type);

            ShaderCompileOptions& withIncludeDirs(const std::vector<std::string>& includeDirs);
            ShaderCompileOptions& withDefines(const std::vector<std::string>& defines);

            friend class Marble::GL::ShaderCompiler;
        private:
            std::vector<std::string> includeDirs;
            std::vector<std::string> defines;
            ShaderType shaderType;
        };

        class coreapi ShaderCompiler final
        {
        public:
            ShaderCompiler() = delete;

            static std::vector<uint8_t> compileShader(const std::string& shaderData, const ShaderCompileOptions& options);
        };
    }
}