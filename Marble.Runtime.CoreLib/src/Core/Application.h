#pragma once

#include <inc.h>

#include <SDL.h>

namespace Marble
{
    class Image;

    class PackageManager;

    namespace Internal
    {
        class CoreEngine;
    }

    class coreapi Application final
    {
        static std::wstring currentWorkingDirectory;
    public:
        static const std::wstring& currentDirectory();

        static int execute(int argc, char* argv[]);
        static void quit();

        friend class Marble::Image;
        friend class Marble::PackageManager;
        friend class Marble::Internal::CoreEngine;
    };
}