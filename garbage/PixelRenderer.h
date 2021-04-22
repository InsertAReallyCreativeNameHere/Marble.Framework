#pragma once

#include <inc.h>

#include <SDL.h>
#include <vector>
#include <atomic>
#include <Core/Mathematics.h>
#include <Rendering/Utils.h>
#include <Extras/Property.h>
#include <Extras/ManagedArray.h>

#if _WIN32
    #include <windows.h>
#endif

using namespace Marble;
using namespace Core::Extras;
using namespace Core::Mathematics;
using namespace Core::Rendering;

namespace Core
{
    namespace Internal
    {
        namespace Rendering
        {
            enum class RenderType : unsigned short int
            {
                Replace,
                Over
            };

            class coreapi PixelRenderer final
            {
                #if _WIN32
                    static HDC deviceContext;
                    static HDC memoryContext;
                    static HBITMAP frameBuffer;
                    static HBRUSH clearBrush;

                    static std::vector<ulong> renderData;
                #endif

                static ManagedArray<byte*> pixels;

                static byte clearColour[3];

                static uint width;
                static uint height;

                // Static class.
                PixelRenderer()
                {

                }
            public:
                #if _WIN32
                    static void init(HDC context);
                #else
                    static void init(void* what);
                #endif
                static void shutdown();

                static void setClearColour(byte const (&col)[3]);

                static void setViewRect(const uint& width, const uint& height);

                static void drawPixel(byte const (&col)[3], const uint& x, const uint& y);
                static void drawPixel(byte const (&col)[4], const uint& x, const uint& y, const RenderType& renderType);
                static void renderAll();
                static void clear();
            };
        }
    }
}