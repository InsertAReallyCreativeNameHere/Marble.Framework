#pragma once

#include "inc.h"

#include <stb_image.h>
#include <Core/PackageManager.h>
#include <Font/Font.h>

namespace Marble
{
    namespace PackageSystem
    {
        class coreapi PortableGraphicPackageFile final : public PackageFile
        {
            stbi_uc* loadedImage;
            int width, height;
            
            inline PortableGraphicPackageFile(std::vector<uint8_t> data)
            {
                int channels;
                this->loadedImage = stbi_load_from_memory(data.data(), data.size(), &this->width, &this->height, &channels, 4);
            }
            inline ~PortableGraphicPackageFile() override
            {
                stbi_image_free(this->loadedImage);
            }
        public:
            inline int imageWidth() { return this->width; };
            inline int imageHeight() { return this->height; };
            inline const uint8_t* imageRGBAData() { return this->loadedImage; };

            friend class Marble::PackageSystem::PackageManager;
        };

        class coreapi TrueTypeFontPackageFile final : public PackageFile
        {
            std::vector<uint8_t> data;
            Typography::Font font;
            
            inline TrueTypeFontPackageFile(std::vector<uint8_t> data) : data(std::move(data)), font((unsigned char*)this->data.data())
            {
            }
        public:
            inline Typography::Font& fontHandle()
            {
                return this->font;
            }

            friend class Marble::PackageSystem::PackageManager;
        };
    }
}