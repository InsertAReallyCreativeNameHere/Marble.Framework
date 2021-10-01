#pragma once

#include "inc.h"
#include "Marble.Runtime.GL.Exports.h"

#include <string>

namespace Marble
{
    namespace GL
    {
        enum class TextureEncodingQuality : uint_fast8_t
        {
            Fastest = 0,
            Default = 1,
            Highest = 2
        };

        class TextureUtility;
        struct TextureCompileOptions final
        {
            inline TextureCompileOptions(std::string fileType) : outputType(std::move(fileType)), outputFormat(""), quality(TextureEncodingQuality::Default)
            {
            }

            inline TextureCompileOptions& withOutputFormat(std::string format)
            {
                this->outputFormat = std::move(format);
                return *this;
            }
            inline TextureCompileOptions& usingQuality(TextureEncodingQuality quality)
            {
                this->quality = quality;
                return *this;
            }

            friend class Marble::GL::TextureUtility;
        private:
            std::string outputType;
            std::string outputFormat;
            TextureEncodingQuality quality;
        };

        class __marble_gl_api TextureUtility final
        {
        public:
            TextureUtility() = delete;

            static std::vector<char> compileTexture(const std::vector<char>& textureData, const TextureCompileOptions& options);
        };
    }
}