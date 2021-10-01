#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

#include <filesystem>

namespace Marble
{
    class Image;

    namespace Internal
    {
        class ShaderUtility;
        class CoreEngine;
    }

    namespace PackageSystem
    {
        class PackageManager;
    }

    class __marble_corelib_api Application final
    {
        static std::filesystem::path currentWorkingDirectory;
    public:
        inline static const std::filesystem::path& currentDirectory()
        {
            return Application::currentWorkingDirectory;
        }

        static int execute(int argc, char* argv[]);
        static void quit();

        friend class Marble::Internal::CoreEngine;
    };
}