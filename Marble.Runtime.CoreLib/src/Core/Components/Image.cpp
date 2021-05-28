#include "Image.h"

#include <stb_image.h>
#include <fstream>
#include <Core/Application.h>
#include <Core/PackageManager.h>
#include <Rendering/Renderer.h>

using namespace Marble;
using namespace Marble::Internal;
using namespace Marble::PackageSystem;
namespace fs = std::filesystem;

std::unordered_map<PortableGraphicPackageFile*, Texture2D> Image::textures;

Image::Image() :
imageFile
(
    []
    {
        return nullptr;
    },
    [this](PortableGraphicPackageFile* file)
    {
        std::string lowerCaseExtension = file->fileLocalPath.extension().string();
        auto ptr = Image::textures.find(file);
        if (ptr != Image::textures.end())
            this->texture = &ptr->second;
        else
        {
            this->texture = &Image::textures[file];
            int w = file->width;
            int h = file->height;

            Texture2D* texture = this->texture;
            /*Renderer::pendingRenderJobs.push_back
            (
                new FunctionRenderJob
                (
                    [=]
                    {
                        texture->internalTexture = SDL_CreateTexture
                        (
                            *Renderer::internalEngineRenderer,
                            #if SDL_BYTEORDER == SDL_LIL_ENDIAN
                                SDL_PIXELFORMAT_ABGR8888,
                            #else
                                SDL_PIXELFORMAT_RGBA8888,
                            #endif
                            SDL_TEXTUREACCESS_STATIC,
                            w,
                            h
                        );
                        SDL_UpdateTexture(texture->internalTexture, null, file->loadedImage, w * 4);
                    },
                    true
                )
            );*/
        }
    }
)
{
}