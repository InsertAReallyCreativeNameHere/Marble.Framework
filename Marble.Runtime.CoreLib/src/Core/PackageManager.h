#pragma once

#include <inc.h>

#include <ctti/nameof.hpp>
#include <filesystem>
#include <list>
#undef STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <unordered_map>
#include <Utility/Hash.h>

void start();

namespace Marble
{
    class Image;

    namespace Internal
    {
        class CoreEngine;
    }

    namespace PackageSystem
    {
        class PackageManager;

        struct coreapi PackageFile
        {
            struct Reflection {
                uint64_t typeID;
            } reflection;

            std::filesystem::path fileLocalPath;

            friend class Marble::Image;
            friend class Marble::PackageSystem::PackageManager;
            friend class Marble::Internal::CoreEngine;
        protected:
            PackageFile(const std::filesystem::path& fileLocalPath, uint64_t typeID);
            virtual ~PackageFile() = 0;
        };

        class coreapi BinaryPackageFile final : public PackageFile
        {
            uint8_t* loadedBytes;
            uint32_t bytesSize;
        public:
            BinaryPackageFile(uint8_t* bytes, uint32_t bytesSize, const std::filesystem::path& fileLocalPath);
            ~BinaryPackageFile() override;
        };
        class coreapi PortableGraphicPackageFile final : public PackageFile
        {
            stbi_uc* loadedImage;
            int width, height;
        public:
            PortableGraphicPackageFile(stbi_uc* imageBytes, int width, int height, const std::filesystem::path& fileLocalPath);
            ~PortableGraphicPackageFile() override;

            inline int imageWidth() { return this->width; };
            inline int imageHeight() { return this->height; };
            inline uint8_t* imageRGBAData() { return this->loadedImage; };

            friend class Marble::Image;
        };

        template <typename T>
        T* file_cast(PackageFile* file)
        {
            static_assert(std::is_base_of<PackageFile, T>::value, "File cast can only work on type \"PackageFile\"!");

            if (file->reflection.typeID == strhash(ctti::nameof<T>().begin()))
                return static_cast<T*>(file);
            else return nullptr;
        }
        
        enum class Endianness : int16_t
        {
            Big = 0,
            Little = 1
        };

        class coreapi PackageManager final
        {
            static Endianness endianness;

            static std::list<PackageSystem::PackageFile*> loadedCorePackage;
            static std::ifstream corePackageStream;
            
            static void normalizePath(std::wstring& path);
            
            static void loadCorePackageIntoMemory(const std::filesystem::path& packagePath);
            static void freeCorePackageInMemory();
        public:
            PackageManager() = delete;

            static PackageSystem::PackageFile* getCorePackageFileByPath(std::wstring filePath);

            friend class Marble::Image;
            friend class Marble::Internal::CoreEngine;
        };
    }
}