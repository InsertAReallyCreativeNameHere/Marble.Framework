#include "TextureUtility.h"

#include <texturec.h>

using namespace Marble::GL;

std::vector<char> TextureUtility::compileTexture(const std::vector<char>& textureData, const TextureCompileOptions& options)
{
    ProfileFunction();

    const char* quality;
    switch (options.quality)
    {
    case TextureEncodingQuality::Fastest:
        quality = "fastest";
        break;
    case TextureEncodingQuality::Default:
        quality = "default";
        break;
    case TextureEncodingQuality::Highest:
        quality = "highest";
        break;
    }

    std::vector<const char*> args
    {
        "-q", quality,
        "--as", options.outputType.c_str()
    };
    
    if (options.outputFormat != "")
    {
        args.push_back("-t");
        args.push_back(options.outputFormat.c_str());
    }

    return ::compileTexture(textureData, args);
}
