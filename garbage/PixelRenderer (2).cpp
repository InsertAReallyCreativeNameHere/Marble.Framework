#include "PixelRenderer.h"

#include <Core/Debug.h>
#define EXPOSE_INTERNALS 1
#include <Core/DsplMgmt.h>
#undef EXPOSE_INTERNALS

using namespace Marble;
using namespace Marble::Internal;

SDL_Window* PixelRenderer::wind;
SDL_Renderer* PixelRenderer::rend;
SDL_Surface* PixelRenderer::windSurf;
SDL_Surface* PixelRenderer::rendSurf;

SDL_Texture* PixelRenderer::rendTex;

byte* PixelRenderer::pixels;
uint PixelRenderer::width;
uint PixelRenderer::height;
int PixelRenderer::pitch;

void PixelRenderer::init(SDL_Renderer* rend)
{
    PixelRenderer::width = 0;
    PixelRenderer::height = 0;
    PixelRenderer::pitch = 0;
    PixelRenderer::rend = rend;
    PixelRenderer::rendTex = SDL_CreateTexture(PixelRenderer::rend, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 1, 1);
}
void PixelRenderer::shutdown()
{
    SDL_DestroyTexture(PixelRenderer::rendTex);
}

void PixelRenderer::begin(const SDL_Rect& r)
{
    SDL_LockTexture(PixelRenderer::rendTex, &r, (void**)&PixelRenderer::pixels, &PixelRenderer::pitch);
}
void PixelRenderer::end()
{
    SDL_UnlockTexture(PixelRenderer::rendTex);
}

void PixelRenderer::drawPixel(const byte (&col)[3], const unsigned int& x, const unsigned int& y)
{
    if (x < PixelRenderer::width && y < PixelRenderer::height)
    {
        byte* targetPixel = PixelRenderer::pixels + y * PixelRenderer::pitch + x * 4;
        *targetPixel = col[0];
        *(targetPixel + 1) = col[1];
        *(targetPixel + 2) = col[2];
        *(targetPixel + 3) = 255u;
    }
}
void PixelRenderer::drawPixel(const byte (&col)[4], const unsigned int& x, const unsigned int& y, const RenderType& renderType = RenderType::Replace)
{
    if (x < PixelRenderer::width && y < PixelRenderer::height)
    {
        byte* targetPixel = PixelRenderer::pixels + y * PixelRenderer::pitch + x * 4;

        if (renderType == RenderType::Over)
        {
            Color blended = Color::alphaBlend
            (
                Color
                (
                    *targetPixel,
                    *(targetPixel + 1),
                    *(targetPixel + 2),
                    255u
                ),
                col
            );

            *targetPixel = blended.r;
            *(targetPixel + 1) = blended.g;
            *(targetPixel + 2) = blended.b;
            *(targetPixel + 3) = 255u;
        }
        else
        {
            *targetPixel = col[0];
            *(targetPixel + 1) = col[1];
            *(targetPixel + 2) = col[2];
            *(targetPixel + 3) = col[3];
        }
    }
}
void PixelRenderer::drawRect(const byte (&col)[3], const uint& x1, const uint& y1, const uint& x2, const uint& y2)
{
    #pragma omp parallel for default(shared) collapse(2)
    for (uint i = x1; i < x2; i++)
    {
        for (uint j = y1; j < y2; j++)
        {
            byte* targetPixel = PixelRenderer::pixels + j * PixelRenderer::pitch + i * 4;
            *targetPixel = col[0];
            *(targetPixel + 1) = col[1];
            *(targetPixel + 2) = col[2];
            *(targetPixel + 3) = 255u;
        }
    }
}

void PixelRenderer::setViewRect(const uint& width, const uint& height)
{
    PixelRenderer::width = width;
    PixelRenderer::height = height;
    PixelRenderer::pitch = PixelRenderer::width * 4;
    SDL_DestroyTexture(PixelRenderer::rendTex);
    PixelRenderer::rendTex = SDL_CreateTexture
    (
        PixelRenderer::rend,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        PixelRenderer::width,
        PixelRenderer::height
    );
}
void PixelRenderer::clear(const byte (&col)[3])
{
    /*#pragma omp parallel for default(shared) collapse(2)
    for (uint i = 0; i < PixelRenderer::width; i++)
    {
        for (uint j = 0; j < PixelRenderer::height; j++)
        {
            byte* targetPixel = PixelRenderer::pixels + j * PixelRenderer::pitch + i * 4;
            *targetPixel = col[0];
            *(targetPixel + 1) = col[1];
            *(targetPixel + 2) = col[2];
            *(targetPixel + 3) = 255u;
        }
    }*/
}

void PixelRenderer::renderAll()
{
    SDL_RenderCopy(PixelRenderer::rend, PixelRenderer::rendTex, null, null);
    SDL_RenderPresent(PixelRenderer::rend);
}