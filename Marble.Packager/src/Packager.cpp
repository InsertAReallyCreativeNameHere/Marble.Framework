#include "Packager.h"

#include <cstdlib>
#include <iostream>
#include <fstream>

#include <filesystem>
namespace fs = std::filesystem;

using namespace Marble::PackageSystem;

Packager::Endianness Packager::endianness;

void Packager::packageFolder(std::wstring_view folder)
{
    std::wstring pakName(fs::current_path().wstring());
    pakName.append(L"/Package.marble.pkg");
    if (fs::exists(pakName.c_str()))
    {
        fputws(L".marble.pkg file exists, removing...\n", stdout);
        fs::remove(pakName.c_str());
        fputws(L"Removed.\n", stdout);
    }

    std::ofstream package(fs::path(pakName.c_str()), std::ios::binary);

    for (auto& path : fs::recursive_directory_iterator(folder))
    {
        if (!fs::is_directory(path.path()))
        {
            std::wcout << L"File - " << path.path().wstring() << L".\n";
            std::wstring curPath(fs::current_path().wstring());
            curPath.append(L"/Package");

            std::wstring filePath = fs::relative(path.path(), folder.data()).wstring();
            uint32_t len = filePath.size();
            uint32_t len_endianConverted = toEndianness(len, Packager::endianness, Packager::Endianness::Big);
            package.write(reinterpret_cast<char*>(&len_endianConverted), sizeof(uint32_t));
            package.write(reinterpret_cast<char*>(&filePath[0]), sizeof(wchar_t) * len);
            std::wcout << L"Relative File Path - " << filePath.c_str() << L".\n";
            
            std::ifstream infile(path.path(), std::ios::binary);

            infile.seekg(0, std::ios::end);
            uint32_t length = infile.tellg();
            wprintf(L"Filesize - %ldB%ls", length, L".\n");
            uint32_t length_endianConverted = toEndianness(length, Packager::endianness, Packager::Endianness::Big);
            package.write(reinterpret_cast<char*>(&length_endianConverted), sizeof(uint32_t));
            infile.seekg(0, std::ios::beg);

            char* buffer = new char[1048576];

            for (uint32_t i = 0; i < length / 1048576; i++)
            {
                fputws(L"Writing 1048576 bytes.\n", stdout);
                infile.read(buffer, 1048576);
                package.write(buffer, sizeof(char) * 1048576);
                fputws(L"Written 1048576 bytes.\n", stdout);
            }

            uint32_t rem = length % 1048576;
            if (rem != 0)
            {
                wprintf(L"Writing %u remaining bytes.\n", rem);
                infile.read(buffer, rem);
                package.write(buffer, sizeof(char) * rem);
                wprintf(L"Written %u remaining bytes.\n", rem);
            }

            delete[] buffer;
        }
    }
}