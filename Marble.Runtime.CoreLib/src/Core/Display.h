#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

#include <atomic>
#include <Mathematics.h>

namespace Marble
{
    namespace Internal
    {
        class CoreEngine;
    }

    class __marble_corelib_api Window final
    {
        static int width;
        static int height;
        static bool resizing;
    public:
        inline static int pixelWidth()
        {
            return Window::width;
        }
        inline static int pixelHeight()
        {
            return Window::height;
        }
        inline static bool isResizing()
        {
            return Window::resizing;
        }

        friend class Marble::Internal::CoreEngine;
    };
    
    class __marble_corelib_api Screen final
    {
        static int width;
        static int height;
        static int screenRefreshRate;
    public:
        inline static int pixelWidth()
        {
            return Screen::width;
        }
        inline static int pixelHeight()
        {
            return Screen::height;
        }
        inline static int refreshRate()
        {
            return Screen::screenRefreshRate;
        }

        friend class Marble::Internal::CoreEngine;
    };
}
