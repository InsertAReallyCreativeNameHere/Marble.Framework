#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <fstream>
#include <Core/Application.h>
#include <Core/PackageManager.h>
#include <Rendering/Core/Renderer.h>

using namespace Marble;
using namespace Marble::Internal;
using namespace Marble::PackageSystem;
using namespace Marble::GL;
namespace fs = std::filesystem;

std::unordered_map<PackageSystem::PortableGraphicPackageFile*, Image::RenderData*> Image::imageTextures;

Image::Image() :
imageFile
({
    []
    {
        return nullptr;
    },
    [this](PortableGraphicPackageFile* file)
    {
        if (this->data != nullptr)
        {
            --this->data->accessCount;
            if (this->data->accessCount == 0)
            {
                Image::imageTextures.erase(this->data->file);
                CoreEngine::pendingRenderJobBatchesOffload.push_back
                (
                    [data = this->data]
                    {
                        data->internalTexture.destroy();
                        delete data;
                    }
                );
            }
        }
        if (file != nullptr)
        {
            auto set = Image::imageTextures.find(file);
            if (set != Image::imageTextures.end())
            {
                this->data = set->second;
                ++this->data->accessCount;
            }
            else
            {
                auto& set = Image::imageTextures[file];
                set = new RenderData { 1, { }, file };
                this->data = set;

                uint32_t width = file->width, height = file->height;
                uint32_t imageDataSize = width * height * 4;
                std::vector<uint8_t> imageData(imageDataSize);
                for (uint32_t i = 0; i < imageDataSize; i++)
                    imageData[i] = file->loadedImage[i];
                
                CoreEngine::pendingRenderJobBatchesOffload.push_back
                (
                    [=, data = this->data, imageData = std::move(imageData)]
                    {
                        data->internalTexture.create(imageData, width, height);
                    }
                );
            }
        }
        else this->data = nullptr;
    }
}),
data(nullptr)
{
}
Image::~Image()
{
    RenderData* data = this->data;
    if (data != nullptr)
    {
        --data->accessCount;
        if (data->accessCount == 0)
        {
            Image::imageTextures.erase(data->file);
            CoreEngine::pendingRenderJobBatchesOffload.push_back
            (
                [=]
                {
                    data->internalTexture.destroy();
                    delete data;
                }
            );
        }
    }
}