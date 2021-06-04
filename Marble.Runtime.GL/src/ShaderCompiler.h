#pragma once

#include <inc.h>

#include <bgfx/bgfx.h>
#include <string_view>

namespace Marble
{
    namespace Internal
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

            friend class Marble::Internal::ShaderCompiler;
        private:
            std::vector<std::string> includeDirs;
            std::vector<std::string> defines;
            ShaderType shaderType;
        };

        class coreapi ShaderCompiler final
        {
            static struct Initializer final {
                Initializer();
            } init;
        public:
            static std::vector<uint8_t> callInternalMain(int argc, const char* argv[]);
            static std::vector<uint8_t> compileShader(const std::string_view& fileName, const ShaderCompileOptions& options);
        };
    }
}