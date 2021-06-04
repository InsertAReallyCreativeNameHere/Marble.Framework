#pragma once

#include <inc.h>

#include <filesystem>

namespace Marble
{
    class Image;

    class PackageManager;

    namespace Internal
    {
        class ShaderCompiler;
        class CoreEngine;
    }

    class coreapi Application final
    {
        static std::filesystem::path currentWorkingDirectory;
    public:
        static const std::filesystem::path& currentDirectory();

        static int execute(int argc, char* argv[]);
        static void quit();

        friend class Marble::Image;
        friend class Marble::PackageManager;
        friend class Marble::Internal::ShaderCompiler;
        friend class Marble::Internal::CoreEngine;
    };
}