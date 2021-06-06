#pragma once

#include <inc.h>

#include <list>
#include <unordered_map>
#include <ctti/nameof.hpp>
#include <Utility/Hash.h>

#include <filesystem>

void start();

namespace Marble
{
    class Image;

    class PackageManager;

    namespace Internal
    {
        class CoreEngine;
    }

    namespace PackageSystem
    {
        struct coreapi PackageFile
        {
            const std::filesystem::path fileLocalPath;
            const uint64_t fileType;

            friend class Marble::Image;
            friend class Marble::PackageManager;
            friend class Marble::Internal::CoreEngine;
        protected:
            PackageFile(const std::filesystem::path& fileLocalPath, uint64_t fileType);
            virtual ~PackageFile() = 0;
        };

        class coreapi BinaryPackageFile final : public PackageFile
        {
        public:
            BinaryPackageFile(const uint8_t* bytes, unsigned bytesSize, const std::filesystem::path& fileLocalPath);
            ~BinaryPackageFile() override;
            
            const uint8_t* loadedBytes;
            const unsigned bytesSize;
        };

        class coreapi PortableGraphicPackageFile final : public PackageFile
        {
            const unsigned imageBytesSize;
        public:
            PortableGraphicPackageFile(uint8_t* imageBytes, int width, int height, const std::filesystem::path& fileLocalPath);
            ~PortableGraphicPackageFile() override;

            uint8_t* loadedImage;
            int width, height;
            
            friend class Marble::Image;
        };

        template <typename T>
        T* file_cast(PackageFile* file)
        {
            static_assert(std::is_base_of<PackageFile, T>::value, "File cast can only work on type \"PackageFile\"!");

            if (file->fileType == strhash(ctti::nameof<T>().begin()))
                return static_cast<T*>(file);
            else return nullptr;
        }
    }

    class coreapi PackageManager final
    {
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

        friend void ::start();
    };
}