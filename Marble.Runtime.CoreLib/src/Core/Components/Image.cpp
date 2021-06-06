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

Image::Image() :
imageFile
({
    []
    {
        return nullptr;
    },
    [this](PortableGraphicPackageFile* file)
    {
        std::string lowerCaseExtension = file->fileLocalPath.extension().string();
    
        RenderData* data = this->data;
        CoreEngine::pendingRenderJobBatches.enqueue
        (
            {
                [=]
                {
                    if (data->internalTexture != nullptr)
                        delete data->internalTexture;
                    if (file != nullptr)
                        data->internalTexture = new Texture2D(file->loadedImage, file->width, file->height);
                }
            }
        );
    }
})
{
    this->data = new RenderData;
}
Image::~Image()
{
    RenderData* data = this->data;
    CoreEngine::pendingRenderJobBatches.enqueue
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