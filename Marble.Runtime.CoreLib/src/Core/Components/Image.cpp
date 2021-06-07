#include "Image.h"

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
                RenderData* data = this->data;
                CoreEngine::pendingRenderJobBatchesOffload.push_back
                (
                    [=]
                    {
                        delete data->internalTexture;
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
                set = new RenderData { 0, nullptr, file };
                this->data = set;

                uint32_t width = file->width, height = file->height;
                uint32_t imageDataSize = width * height * 4;
                uint8_t* imageData = new uint8_t[imageDataSize];
                for (uint32_t i = 0; i < imageDataSize; i++)
                    imageData[i] = file->loadedImage[i];
                
                RenderData* data = this->data;
                CoreEngine::pendingRenderJobBatchesOffload.push_back
                (
                    [=]
                    {
                        data->internalTexture = new Texture2D(imageData, width, height);
                        delete[] imageData;
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
    CoreEngine::pendingRenderJobBatchesOffload.push_back
    (
        {
            [=]
            {
                if (data->internalTexture != nullptr)
                    delete data->internalTexture;
                delete data;
            }
        }
    );
}