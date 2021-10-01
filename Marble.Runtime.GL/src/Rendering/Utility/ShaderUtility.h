#pragma once

#include "inc.h"
#include "Marble.Runtime.GL.Exports.h"

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
        struct ShaderCompileOptions final
        {
            inline ShaderCompileOptions(ShaderType type)
            {
                this->shaderType = type;
            }

            inline ShaderCompileOptions& withIncludeDirs(const std::vector<std::string>& includeDirs)
            {
                this->includeDirs.insert(this->includeDirs.end(), includeDirs.begin(), includeDirs.end());
                return *this;
            }
            inline ShaderCompileOptions& withDefines(const std::vector<std::string>& defines)
            {
                this->defines.insert(this->defines.end(), defines.begin(), defines.end());
                return *this;
            }

            friend class Marble::GL::ShaderUtility;
        private:
            std::vector<std::string> includeDirs;
            std::vector<std::string> defines;
            ShaderType shaderType;
        };

        class __marble_gl_api ShaderUtility final
        {
        public:
            ShaderUtility() = delete;

            static std::vector<char> compileShader(const std::string& shaderData, const ShaderCompileOptions& options);
        };
    }
}