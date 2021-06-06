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

        class ShaderUtility;
        struct coreapi ShaderCompileOptions final
        {
            ShaderCompileOptions(ShaderType type);

            ShaderCompileOptions& withIncludeDirs(const std::vector<std::string>& includeDirs);
            ShaderCompileOptions& withDefines(const std::vector<std::string>& defines);

            friend class Marble::GL::ShaderUtility;
        private:
            std::vector<std::string> includeDirs;
            std::vector<std::string> defines;
            ShaderType shaderType;
        };

        class coreapi ShaderUtility final
        {
        public:
            ShaderUtility() = delete;

            static std::vector<char> compileShader(const std::string& shaderData, const ShaderCompileOptions& options);
        };
    }
}