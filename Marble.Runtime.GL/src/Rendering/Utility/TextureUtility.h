#pragma once

#include "inc.h"

#include <string>

namespace Marble
{
    namespace GL
    {
        enum class TextureEncodingQuality : uint8_t
        {
            Fastest = 0,
            Default = 1,
            Highest = 2
        };

        class TextureUtility;
        struct coreapi TextureCompileOptions final
        {
            TextureCompileOptions(const std::string& fileType);

            TextureCompileOptions& withOutputFormat(const std::string& format);
            TextureCompileOptions& usingQuality(TextureEncodingQuality quality);

            friend class Marble::GL::TextureUtility;
        private:
            std::string outputType;
            std::string outputFormat;
            TextureEncodingQuality quality;
        };

        class coreapi TextureUtility final
        {
        public:
            TextureUtility() = delete;

            static std::vector<char> compileTexture(const std::vector<char>& textureData, const TextureCompileOptions& options);
        };
    }
}