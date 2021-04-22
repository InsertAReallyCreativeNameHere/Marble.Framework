#pragma once

#include <inc.h>

#include <atomic>
#include <Mathematics.h>

namespace Marble
{
    namespace Internal
    {
        class CoreEngine;
    }

    class coreapi Window final
    {
        static int width;
        static int height;

        static std::atomic<bool> resizing;
    public:
        static int pixelWidth();
        static int pixelHeight();

        static bool isResizing();

        friend class Marble::Internal::CoreEngine;
    };
    class coreapi Screen final
    {
        static int width;
        static int height;
        static int screenRefreshRate;
    public:
        static int pixelWidth();
        static int pixelHeight();
        static int refreshRate();

        friend class Marble::Internal::CoreEngine;
    };
}
