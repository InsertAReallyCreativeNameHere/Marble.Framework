#pragma once

#include <inc.h>

#include <SDL.h>
#include <SDL_syswm.h>
#include <array>
#include <atomic>
#include <Maths/Mathematics.h>
#include <Rendering/Utils.h>
#include <Extras/ManagedArray.h>

#if 0
    #include <wingdi.h>

    #define WinRGBInt(r, g, b) RGB(r, g, b)
#endif

namespace Marble
{
    namespace Internal
    {
        enum class RenderType : unsigned short int
        {
            Replace,
            Over
        };

        class coreapi PixelRenderer final
        {
        public:
            static SDL_Window* wind;
            static SDL_Renderer* rend;
            static SDL_Surface* windSurf;
            static SDL_Surface* rendSurf;

            static SDL_Texture* rendTex;
            static byte* pixels;

            static uint width;
            static uint height;
            static int pitch;
            
            // This is an internal header, the release one will not have all public stuff.
            PixelRenderer() = delete;

            static void init(SDL_Renderer* rend);
            static void shutdown();

            static void begin(const SDL_Rect& r = { 0, 0, static_cast<int>(PixelRenderer::width), static_cast<int>(PixelRenderer::height) });
            static void end();

            static void drawPixel(const byte (&col)[3], const uint& x, const uint& y);
            static void drawPixel(const byte (&col)[4], const uint& x, const uint& y, const RenderType& renderType);
            static void drawRect(const byte (&col)[3], const uint& x1, const uint& y1, const uint& x2, const uint& y2);

            static void setViewRect(const uint& width, const uint& height);
            static void clear(const byte (&col)[3]);

            static void renderAll();
        };

        #define UnsafePixelRendererWidth (PixelRenderer::rendSurf->w)
        #define UnsafePixelRendererHeight (PixelRenderer::rendSurf->h)
    }
}

#include <Core/CoreEngine.h>