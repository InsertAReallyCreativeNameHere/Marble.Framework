#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>

#define Stringify(x) #x
#define StringifyValueOf(x) Stringify(x)

namespace fs = std::filesystem;
//                              .sc File        .mpsh File
// Usage: Marble.ShaderCompiler [Shader Source] [Output Binary] [Shader Type (v/f/c)] [Additional BGFX shaderc Arguments]...
int main(int argc, char* argv[])
{
    std::string shadercExec = "shaderc" StringifyValueOf(CMAKE_EXECUTABLE_SUFFIX);
    if (!fs::exists(shadercExec))
    {
        std::cout << "The BGFX shader compiler (shaderc) could not be found in the current directory or PATH.";
        return EXIT_FAILURE;
    }

    if (argc < 4)
    {
        std::cout <<
        "The input shader source, output binary, and shader type must be specified!\n"
        "Usage: Marble.ShaderCompiler [Shader Source] [Output Binary] [Shader Type (v/f/c)] [Additional BGFX shaderc Arguments]...\n";
        return EXIT_FAILURE;
    }

    std::string source = argv[1];
    std::string out = argv[2];
    char shaderType = argv[3][0];
    
    std::string shadercArgs;
    shadercArgs.append("-f ").append(source);
    shadercArgs.push_back(' ');
    shadercArgs.append("--type ").push_back(shaderType);
    shadercArgs.push_back(' ');
    for (int i = 4; i < argc; i++)
    {
        shadercArgs.append(argv[i]);
        shadercArgs.push_back(' ');
    }

    std::string shadercExecStr = shadercExec;
    std::string cmdLineExec;
    cmdLineExec.push_back('\"');
    cmdLineExec.append(shadercExecStr).append("\" -p ");
    size_t resetToSize = shadercExecStr.size() + 6;

    #if _WIN32
    switch (shaderType) // NB: d3d9.
    {
    case 'v':
        cmdLineExec.append("vs_3_0 ");
        break;
    case 'f':
    case 'c':
        cmdLineExec.append("ps_3_0 ");
        break;
    }
    cmdLineExec.append("-o ").append(out).append(".d3d9.bin ");
    cmdLineExec.append(shadercArgs);
    std::cout << "Command line execution for d3d9: " << cmdLineExec << '\n';
    system(cmdLineExec.c_str());

    cmdLineExec.resize(resetToSize);
    switch (shaderType) // NB: d3d11.
    {
    case 'v':
        cmdLineExec.append("vs_4_0 ");
        break;
    case 'f':
        cmdLineExec.append("ps_4_0 ");
        break;
    case 'c':
        cmdLineExec.append("cs_5_0 ");
        break;
    }
    cmdLineExec.append("-o ").append(out).append(".d3d11.bin ");
    cmdLineExec.append(shadercArgs);
    std::cout << "Command line execution for d3d11: " << cmdLineExec << '\n';
    system(cmdLineExec.c_str());
    
    cmdLineExec.resize(resetToSize);
    switch (shaderType) // NB: d3d12.
    {
    case 'v':
        cmdLineExec.append("vs_5_0 ");
        break;
    case 'f':
        cmdLineExec.append("ps_5_0 ");
        break;
    case 'c':
        cmdLineExec.append("cs_5_0 ");
        break;
    }
    cmdLineExec.append("-o ").append(out).append(".d3d12.bin ");
    cmdLineExec.append(shadercArgs);
    std::cout << "Command line execution for d3d12: " << cmdLineExec << '\n';
    system(cmdLineExec.c_str());
    #endif
    
    cmdLineExec.resize(resetToSize);
    switch (shaderType) // NB: OpenGL.
    {
    case 'v':
    case 'f':
        cmdLineExec.append("120 ");
        break;
    case 'c':
        cmdLineExec.append("430 ");
        break;
    }
    cmdLineExec.append("-o ").append(out).append(".opengl.bin ");
    cmdLineExec.append(shadercArgs);
    std::cout << "Command line execution for OpenGL: " << cmdLineExec << '\n';
    system(cmdLineExec.c_str());
    
    cmdLineExec.resize(resetToSize);
    cmdLineExec.append("spirv10-10 "); // NB: Vulkan.
    cmdLineExec.append("-o ").append(out).append(".vulkan.bin ");
    cmdLineExec.append(shadercArgs);
    std::cout << "Command line execution for Vulkan: " << cmdLineExec << '\n';
    system(cmdLineExec.c_str());

    // TODO: Implement other rendering backends as well.

    std::ofstream outFile(out, std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(u8"psh"), sizeof(char8_t) * 3);
    // NB: File format version 0x__MMmmpp -> _ Unused, M Major, m Minor, p Patch.
    uint32_t formatVersion = 0x00000100;
    outFile.write(reinterpret_cast<const char*>(&formatVersion), sizeof(uint32_t));
    
    auto writeBinary = [&](fs::path binChunk, std::u8string header) -> void
    {
        if (fs::exists(binChunk))
        {
            uint64_t headerSize = uint64_t(sizeof(char8_t) * header.size());
            outFile.write(reinterpret_cast<const char*>(&headerSize), sizeof(uint64_t));
            outFile.write(reinterpret_cast<const char*>(header.c_str()), sizeof(char8_t) * header.size());
            std::ifstream inFile(binChunk, std::ios::binary);
            uint64_t dataSize = fs::file_size(binChunk);
            char* pshData = new char[dataSize];
            inFile.read(pshData, dataSize);
            outFile.write(reinterpret_cast<const char*>(&dataSize), sizeof(uint64_t));
            outFile.write(pshData, dataSize);
        }
    };
    #if _WIN32
    writeBinary(out + ".d3d9.bin", u8"d3d9");
    writeBinary(out + ".d3d11.bin", u8"d3d11");
    writeBinary(out + ".d3d12.bin", u8"d3d12");
    #endif
    writeBinary(out + ".opengl.bin", u8"opengl");
    writeBinary(out + ".vulkan.bin", u8"vulkan");
}