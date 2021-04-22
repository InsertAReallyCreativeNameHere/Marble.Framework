#include <cstdio>
#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <io.h>

#include <filesystem>
namespace fs = std::filesystem;

#include <Packager.h>

// Thanks random website. Very cool. https://hbfs.wordpress.com/2017/01/10/strings-in-buffer-switchcase-statements/.
uint64_t constexpr strmix(wchar_t m, uint64_t s)
{
    return ((s<<7) + ~(s>>3)) + ~m;
}
uint64_t constexpr strhash(const wchar_t* m)
{
    return (*m) ? strmix(*m,strhash(m+1)) : 0;
}

int main(int argc, char* argv[])
{
    _setmode(_fileno(stdout), _O_U8TEXT);
    fputws(L"Welcome to packager! Type \"help\" to get help.\n", stdout);
    wprintf(L"Current path is %ls%ls", fs::current_path().wstring().c_str(), L".\n");

    std::wstring input;   
    while (true)
    {
        std::wcin >> input;

        switch (strhash(input.c_str()))
        {
        case strhash(L"exit"):
            {
                fputws(L"Alrighty! Exiting...\n", stdout);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                goto Exit;
            }
            break;
        case strhash(L"help"):
            {
                fputws
                (
LR"(Commands:
    pack - Will pack the "Package" folder in the same folder as this executable into a formatted file for the Marble Runtime Environment.
    unpack - Will unpack the package into a zip file.
)",
                    stdout
                );
            }
            break;
        case strhash(L"pack"):
            {
                fputws(L"Alrighty! Packing folder into a binary file.\n", stdout);
                std::wstring assetsFolder(fs::current_path().c_str());
                assetsFolder.append(L"/Package");
                if (!fs::exists(assetsFolder.c_str()))
                {
                    fputws(L"The \"Package\" folder doesn't exist, create one!\n", stderr);
                    break;
                }
                Packager::packageFolder(assetsFolder.c_str());
                
                std::fstream file("Package.marble.pkg", std::ios::binary);
                file.seekg(0, std::ios::end);
                size_t length = file.tellg();
                file.seekg(0, std::ios::beg);
                
                char* buffer = new char[16384];

                #pragma omp parallel for default(none) shared(buffer, length, file)
                for (unsigned i = 0; i < length / 16384; i++)
                {
                    file.read(buffer, 16384);

                    for (unsigned j = 0; j < 16384; j++)
                    {
                        buffer[j] = 255u - buffer[j];
                    }
                    
                    file.write(buffer, 16384);
                }

                int rem = length % 16384;
                file.read(buffer, rem);

                #pragma omp parallel for default(none) shared(buffer, rem)
                for (unsigned i = 0; i < rem; i++)
                {
                    buffer[i] = 255u - buffer[i];
                }
                
                file.write(buffer, rem);

                fputws(L"Done.\n", stdout);
            }
            break;
        case strhash(L"unpack"):
            {
                fputws(L"Alrighty! Unpacking folder into a zip file.\n", stdout);

                std::wstring curDir(fs::current_path().wstring());
                if (fs::exists((curDir + L"/Package.marble.pkg").c_str()))
                {
                    if (fs::exists((curDir + L"/Package_Unpacked").c_str()))
                        fs::remove_all((curDir + L"/Package_Unpacked").c_str());
                    fs::create_directory(curDir + L"/Package_Unpacked");

                    std::ifstream package("Package.marble.pkg", std::ios::binary);
                    package.seekg(0, std::ios::end);
                    unsigned length = package.tellg();
                    package.seekg(0, std::ios::beg);

                    char* buffer = new char[16384];

                    while (package.tellg() < length)
                    {
                        unsigned filePathLength;
                        package.read(reinterpret_cast<char*>(&filePathLength), sizeof(unsigned));

                        std::wstring filePath(curDir + L"/Package_Unpacked/");
                        std::wstring relPath;
                        relPath.resize(filePathLength);
                        package.read(reinterpret_cast<char*>(&relPath[0]), sizeof(wchar_t) * filePathLength);
                        filePath.append(relPath);

                        wprintf(L"File Path - %ls.\n", filePath.c_str());

                        unsigned fileLen;
                        package.read(reinterpret_cast<char*>(&fileLen), sizeof(unsigned));
                        
                        fs::create_directories(fs::path(filePath.c_str()).parent_path());
                        std::ofstream outFile(filePath.c_str(), std::ios::binary);

                        for (unsigned i = 0; i < fileLen / 16384; i++)
                        {
                            package.read(buffer, 16384);
                            outFile.write(buffer, sizeof(char) * 16384);
                        }

                        int rem = fileLen % 16384;
                        package.read(buffer, rem);
                        outFile.write(buffer, sizeof(char) * rem);
                    }

                    delete[] buffer;

                    fputws(L"Done.\n", stdout);
                }
                else fputws(L"Packaged file doesn't exist. Use the \"pack\" command to create one!\n", stderr);
            }
            break;
        default:
            fputws(L"Unknown command, use \"help\" to get a list of commands.\n", stdout);
        }
    }

    Exit:;
}