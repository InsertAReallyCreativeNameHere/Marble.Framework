#include "Image.h"

#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <Mathematics.h>
#include <Core/Application.h>
#include <Core/Components/RectTransform.h>
#include <Core/PackageManager.h>
#include <Rendering/Core/Renderer.h>

using namespace Marble;
using namespace Marble::Internal;
using namespace Marble::Mathematics;
using namespace Marble::GL;
using namespace Marble::PackageSystem;
namespace fs = std::filesystem;

robin_hood::unordered_map<PackageSystem::PortableGraphicPackageFile*, Image::RenderData*> Image::imageTextures;

Image::~Image()
{
    ProfileFunction();
    
    RenderData* data = this->data;
    if (data)
    {
        --data->accessCount;
        if (data->accessCount == 0)
        {
            Image::imageTextures.erase(data->file);
            CoreEngine::queueRenderJobForFrame
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

void Image::setImageFile(PortableGraphicPackageFile* value)
{
    ProfileFunction();
    
    if (this->data)
    {
        --this->data->accessCount;
        if (this->data->accessCount == 0)
        {
            Image::imageTextures.erase(this->data->file);
            CoreEngine::queueRenderJobForFrame
            (
                [data = this->data]
                {
                    data->internalTexture.destroy();
                    delete data;
                }
            );
        }
    }
    if (value)
    {
        auto set = Image::imageTextures.find(value);
        if (set != Image::imageTextures.end())
        {
            this->data = set->second;
            ++this->data->accessCount;
        }
        else
        {
            this->data = Image::imageTextures.insert(robin_hood::pair<PortableGraphicPackageFile*, RenderData*>(value, new RenderData { 1, { }, value })).first->second;

            uint32_t width = value->imageWidth(), height = value->imageHeight();
            uint32_t imageDataSize = width * height * 4;
            std::vector<uint8_t> imageData(value->imageRGBAData(), value->imageRGBAData() + imageDataSize);
            
            CoreEngine::queueRenderJobForFrame
            (
                [width, height, data = this->data, imageData = std::move(imageData)]
                {
                    data->internalTexture.create(imageData, width, height);
                }
            );
        }
    }
    else this->data = nullptr;
}

void Image::renderOffload()
{
    ProfileFunction();
    
    if (this->data)
    {
        RectTransform* thisRect = this->rectTransform();
        const Vector2& pos = thisRect->position;
        const Vector2& scale = thisRect->scale;
        const RectFloat& rect = thisRect->rect;

        ColoredTransformHandle t;
        t.setPosition(pos.x, pos.y);
        t.setOffset((rect.right + rect.left) / 2, (rect.top + rect.bottom) / 2);
        t.setScale(scale.x * (rect.right - rect.left), scale.y * (rect.top - rect.bottom));
        t.setRotation(deg2RadF(thisRect->rotation));
        t.setColor(1.0f, 1.0f, 1.0f, 1.0f);
        
        CoreEngine::queueRenderJobForFrame
        (
            [t, data = this->data]
            {
                Renderer::drawImage(data->internalTexture, t);
            }
        );
    }
}
