#include "Packager.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <zip.h>

#include <filesystem>
namespace fs = std::filesystem;

void Packager::packageFolder(const std::wstring_view& folder)
{
    std::wstring pakName(fs::current_path().wstring());
    pakName.append(L"/Package.marble.pkg");
    if (fs::exists(pakName.c_str()))
    {
        fputws(L".marble.pkg file exists, removing...\n", stdout);
        fs::remove(pakName.c_str());
        fputws(L"Removed.\n", stdout);
    }

    std::ofstream package(pakName.c_str(), std::ios::binary);

    for (auto& path : fs::recursive_directory_iterator(folder))
    {
        if (!fs::is_directory(path.path()))
        {
            std::wcout << L"File - " << path.path().wstring() << L".\n";
            std::wstring curPath(fs::current_path().wstring());
            curPath.append(L"/Package");

            std::wstring filePath = fs::relative(path.path(), curPath.c_str()).wstring();
            unsigned len = filePath.size();
            package.write(reinterpret_cast<char*>(&len), sizeof(unsigned));
            package.write(reinterpret_cast<char*>(&filePath[0]), sizeof(wchar_t) * len);
            std::wcout << L"Relative File Path - " << filePath.c_str() << L".\n";
            
            std::ifstream infile(path.path().wstring().c_str(), std::ios::binary);

            infile.seekg(0, std::ios::end);
            unsigned length = infile.tellg();
            wprintf(L"Filesize - %ldB%ls", length, L".\n");
            package.write(reinterpret_cast<char*>(&length), sizeof(unsigned));
            infile.seekg(0, std::ios::beg);

            char* buffer = new char[8388608];

            for (unsigned i = 0; i < length / 8388608; i++)
            {
                fputws(L"Writing 1048576 bytes.\n", stdout);
                infile.read(buffer, 8388608);
                package.write(buffer, sizeof(char) * 8388608);
                fputws(L"Written 1048576 bytes.\n", stdout);
            }

            int rem = length % 8388608;
            infile.read(buffer, rem);
            package.write(buffer, sizeof(char) * rem);

            delete[] buffer;
        }
    }
}