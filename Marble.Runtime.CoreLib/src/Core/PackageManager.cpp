#include "PackageManager.h"

#include <cctype>
#include <cmath>
#include <fstream>
#include <stb_image.h>
#include <Core/Application.h>

using namespace Marble;
using namespace Marble::PackageSystem;
namespace fs = std::filesystem;

PackageFile::PackageFile(const std::filesystem::path& fileLocalPath, uint64_t typeID) : fileLocalPath(fileLocalPath), reflection { typeID }
{
}
PackageFile::~PackageFile()
{
}

BinaryPackageFile::BinaryPackageFile(uint8_t* bytes, uint32_t bytesSize, const fs::path& fileLocalPath) :
PackageFile(fileLocalPath, strhash(ctti::nameof<BinaryPackageFile>().begin())), loadedBytes(bytes), bytesSize(bytesSize)
{
}
BinaryPackageFile::~BinaryPackageFile()
{
    delete[] this->loadedBytes;
}

PortableGraphicPackageFile::PortableGraphicPackageFile(stbi_uc* imageBytes, int width, int height, const fs::path& fileLocalPath) :
PackageFile(fileLocalPath, strhash(ctti::nameof<PortableGraphicPackageFile>().begin())), loadedImage(imageBytes), width(width), height(height)
{
}
PortableGraphicPackageFile::~PortableGraphicPackageFile()
{
    stbi_image_free(const_cast<stbi_uc*>(this->loadedImage));
}

constexpr auto toEndianness = [](auto intType, Endianness from, Endianness to) -> decltype(intType)
{
    if (from == to)
        return intType;
    else
    {
        decltype(intType) _int = 0;
        for (size_t i = 0; i < sizeof(decltype(intType)); i++)
            reinterpret_cast<int8_t*>(&_int)[i] = reinterpret_cast<int8_t*>(&intType)[sizeof(decltype(intType)) - 1 - i];
        return _int;
    }
};

Endianness PackageManager::endianness;

std::list<PackageFile*> PackageManager::loadedCorePackage;
std::ifstream PackageManager::corePackageStream;

void PackageManager::loadCorePackageIntoMemory(const fs::path& packagePath)
{
    PackageManager::corePackageStream.open(packagePath, std::ios::binary);
    PackageManager::corePackageStream.seekg(0, std::ios::end);
    uint32_t length = PackageManager::corePackageStream.tellg();
    PackageManager::corePackageStream.seekg(0, std::ios::beg);

    while (PackageManager::corePackageStream.tellg() < length)
    {
        uint32_t filePathLength_endianUnconverted;
        PackageManager::corePackageStream.read(reinterpret_cast<char*>(&filePathLength_endianUnconverted), sizeof(uint32_t));
        uint32_t filePathLength = toEndianness(filePathLength_endianUnconverted, Endianness::Big, PackageManager::endianness);

        std::wstring filePath;
        filePath.resize(filePathLength);
        PackageManager::corePackageStream.read(reinterpret_cast<char*>(&filePath[0]), sizeof(wchar_t) * filePathLength);
        PackageManager::normalizePath(filePath);
        Debug::LogTrace("Package File - ", filePath, ".");

        uint32_t fileLen_endianUnconverted;
        PackageManager::corePackageStream.read(reinterpret_cast<char*>(&fileLen_endianUnconverted), sizeof(uint32_t));
        uint32_t fileLen = toEndianness(fileLen_endianUnconverted, Endianness::Big, PackageManager::endianness);
        Debug::LogTrace("Package File Size - ", fileLen, "B.");

        uint8_t* fileBytes = new uint8_t[fileLen];
        PackageManager::corePackageStream.read(reinterpret_cast<char*>(fileBytes), fileLen);

        switch (wstrhash(fs::path(filePath).extension().wstring().c_str()))
        {
        case wstrhash(L".png"):
            {
                int w, h, channels;
                stbi_uc* loadedImage = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(fileBytes), fileLen, &w, &h, &channels, 4);
                delete[] fileBytes;

                PackageManager::loadedCorePackage.push_back(new PortableGraphicPackageFile(loadedImage, w, h, filePath));
            }
            break;
        default:
            PackageManager::loadedCorePackage.push_back(new BinaryPackageFile(fileBytes, fileLen, filePath));
        }
    }
}
void PackageManager::freeCorePackageInMemory()
{
    PackageManager::corePackageStream.close();
    for (auto it = PackageManager::loadedCorePackage.begin(); it != PackageManager::loadedCorePackage.end(); ++it)
        delete *it;
}

void PackageManager::normalizePath(std::wstring& path)
{
    bool extensionEnded = false;
    for (auto it = path.rbegin(); it != path.rend(); ++it)
    {
        switch (*it)
        {
        case L'\\':
            *it = L'/';
            break;
        case L'.':
            extensionEnded = true;
            break;
        }
        if (!extensionEnded)
            *it = std::tolower(*it);
    }
}
PackageFile* PackageManager::getCorePackageFileByPath(std::wstring filePath)
{
    PackageManager::normalizePath(filePath);
    for (auto it = PackageManager::loadedCorePackage.begin(); it != PackageManager::loadedCorePackage.end(); ++it)
    {
        Debug::LogInfo((*it)->fileLocalPath, "  ", filePath);
        if ((*it)->fileLocalPath == filePath)
            return *it;
    }
    return nullptr;
}