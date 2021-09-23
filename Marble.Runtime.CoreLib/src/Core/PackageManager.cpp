#include "PackageManager.h"

#include <cctype>
#include <cmath>
#include <fstream>
#include <Core/Application.h>
#include <Core/Debug.h>
#include <Utility/TypeInfo.h>

using namespace Marble;
using namespace Marble::PackageSystem;
namespace fs = std::filesystem;

constexpr auto toEndianness = [](auto intType, Endianness from, Endianness to) constexpr -> decltype(intType)
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

void PackageManager::removeBinaryFileHandler(const std::vector<uint8_t>& signature, size_t offset)
{
    auto it1 = PackageManager::binFileHandlers.find(offset);
    if (it1 != PackageManager::binFileHandlers.end())
    {
        auto it2 = it1->second.find(signature.size());
        if (it2 != it1->second.end())
        {
            it2->second.erase(std::basic_string<uint8_t>(signature.data(), signature.size()));
            if (it2->second.empty())
            {
                it1->second.erase(it2);
                if (it1->second.empty())
                    PackageManager::binFileHandlers.erase(it1);
            }
        }
        
    }
}

std::map<std::streamoff, std::map<size_t, robin_hood::unordered_map<std::basic_string<uint8_t>, PackageFile* (*)(std::vector<uint8_t>)>, std::greater<size_t>>> PackageManager::binFileHandlers;
robin_hood::unordered_map<std::wstring, PackageFile* (*)(std::u32string)> PackageManager::textFileHandlers;

std::vector<PackageFile*> PackageManager::loadedCorePackage;
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

        std::vector<uint8_t> fileBytes(fileLen);
        PackageManager::corePackageStream.read(reinterpret_cast<char*>(fileBytes.data()), fileLen);

        for (auto it1 = PackageManager::binFileHandlers.begin(); it1 != PackageManager::binFileHandlers.end(); ++it1)
        {
            for (auto it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
            {
                std::basic_string<uint8_t> sig(fileBytes.data() + it1->first, fileBytes.data() + it1->first + it2->first);
                auto it3 = it2->second.find(sig);
                if (it3 != it2->second.end())
                {
                    PackageFile* file = it3->second(std::move(fileBytes));
                    file->fileLocalPath = filePath;
                    PackageManager::loadedCorePackage.push_back(file);
                    goto FileHandled;
                }
            }
        }
        if
        (
            auto it = PackageManager::textFileHandlers.find(fs::path(filePath).extension().wstring().c_str());
            it != PackageManager::textFileHandlers.end()
        )
        {
            Debug::LogError("Text file loading has not been implemented. This file handler will not be run.");
        }
        else PackageManager::loadedCorePackage.push_back(new BinaryPackageFile(std::move(fileBytes), filePath));

        FileHandled:;
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
        if ((*it)->fileLocalPath == filePath)
            return *it;
    }
    return nullptr;
}