#include "PixelRenderer.h"

#include <math.h>
#include <bgfx/bgfx.h>
#include <Core/Debug.h>
#include <Core/DsplMgmt.h>

#include <Core/Debug.h>

using namespace Core;
using namespace Core::Internal::Rendering;
using namespace Core::Internal::DisplayManagement;

#if _WIN32
    HDC PixelRenderer::deviceContext;
    HDC PixelRenderer::memoryContext;
    HBITMAP PixelRenderer::frameBuffer;
    HBRUSH PixelRenderer::clearBrush;

    std::vector<ulong> PixelRenderer::renderData(0);
#endif

ManagedArray<byte*> PixelRenderer::pixels(0);

byte PixelRenderer::clearColour[3];

uint PixelRenderer::width;
uint PixelRenderer::height;

#if _WIN32
    void PixelRenderer::init(HDC context)
    {
        PixelRenderer::width = 0;
        PixelRenderer::height = 0;
        PixelRenderer::pixels = ManagedArray<byte*>(0, nullptr);
        
        PixelRenderer::deviceContext = context;
        PixelRenderer::memoryContext = CreateCompatibleDC(PixelRenderer::deviceContext);
        PixelRenderer::clearBrush = CreateSolidBrush(RGB(0, 0, 0));
    }
#else
    void PixelRenderer::init(void* what)
    {

    }
#endif
void PixelRenderer::shutdown()
{
    DeleteDC(PixelRenderer::memoryContext);
    DeleteObject(PixelRenderer::clearBrush);
}

void PixelRenderer::setClearColour(byte const (&col)[3])
{
    PixelRenderer::clearColour[0] = col[0];
    PixelRenderer::clearColour[1] = col[1];
    PixelRenderer::clearColour[2] = col[2];
    
    DeleteObject(PixelRenderer::clearBrush);
    PixelRenderer::clearBrush = CreateSolidBrush(((COLORREF)(((BYTE)(PixelRenderer::clearColour[0])|((WORD)((BYTE)(PixelRenderer::clearColour[1]))<<8))|(((DWORD)(BYTE)(PixelRenderer::clearColour[2]))<<16))));
}

void PixelRenderer::setViewRect(const uint& width, const uint& height)
{
    #if _WIN32
        PixelRenderer::width = width;
        PixelRenderer::height = height;
        #pragma omp parallel for default(shared) num_threads(2)
        for (uint i = 0; i < PixelRenderer::pixels.length(); i++)
            delete[] PixelRenderer::pixels[i];
        PixelRenderer::pixels = ManagedArray<byte*>(width * height);
        #pragma omp parallel for default(shared) num_threads(2)
        for (uint i = 0; i < PixelRenderer::pixels.length(); i++)
        {
            PixelRenderer::pixels[i] = new byte[4];
            PixelRenderer::pixels[i][0] = PixelRenderer::clearColour[0];
            PixelRenderer::pixels[i][1] = PixelRenderer::clearColour[1];
            PixelRenderer::pixels[i][2] = PixelRenderer::clearColour[2];
            PixelRenderer::pixels[i][3] = 255u;
        }
    #else
    #endif
}

void PixelRenderer::drawPixel(byte const (&col)[3], const unsigned int& x, const unsigned int& y)
{
    if (x < PixelRenderer::width && y < PixelRenderer::height)
    {
        PixelRenderer::pixels[y * PixelRenderer::width + x][0] = col[0];
        PixelRenderer::pixels[y * PixelRenderer::width + x][1] = col[1];
        PixelRenderer::pixels[y * PixelRenderer::width + x][2] = col[2];
        PixelRenderer::pixels[y * PixelRenderer::width + x][3] = 255u;
    }
}
void PixelRenderer::drawPixel(byte const (&col)[4], const unsigned int& x, const unsigned int& y, const RenderType& renderType)
{
    if (x < PixelRenderer::width && y < PixelRenderer::height)
    {
        if (renderType == RenderType::Over)
        {
            Color blended = Color::alphaBlend
            (
                Color
                (
                    PixelRenderer::pixels[y * PixelRenderer::width + x][0],
                    PixelRenderer::pixels[y * PixelRenderer::width + x][1],
                    PixelRenderer::pixels[y * PixelRenderer::width + x][2],
                    255u
                ),
                col
            );

            PixelRenderer::pixels[y * PixelRenderer::width + x][0] = blended.r;
            PixelRenderer::pixels[y * PixelRenderer::width + x][1] = blended.g;
            PixelRenderer::pixels[y * PixelRenderer::width + x][2] = blended.b;
            PixelRenderer::pixels[y * PixelRenderer::width + x][3] = 255u;
        }
        else
        {
            PixelRenderer::pixels[y * PixelRenderer::width + x][0] = col[0];
            PixelRenderer::pixels[y * PixelRenderer::width + x][1] = col[1];
            PixelRenderer::pixels[y * PixelRenderer::width + x][2] = col[2];
            PixelRenderer::pixels[y * PixelRenderer::width + x][3] = col[3];
        }
    }
}
void PixelRenderer::renderAll()
{
    uint count = PixelRenderer::width * PixelRenderer::height;
    PixelRenderer::renderData.resize(count);
    for (int i = 0; i < count; i++)
    {
        PixelRenderer::renderData[i] = RGB
        (
            (static_cast<ulong>(PixelRenderer::pixels[i][2]) * static_cast<ulong>(PixelRenderer::pixels[i][3] / 255)),
            (static_cast<ulong>(PixelRenderer::pixels[i][1]) * static_cast<ulong>(PixelRenderer::pixels[i][3] / 255)),
            (static_cast<ulong>(PixelRenderer::pixels[i][0]) * static_cast<ulong>(PixelRenderer::pixels[i][3] / 255))
        );
        //(((static_cast<ulong>(PixelRenderer::pixels[i][0]) * static_cast<ulong>(PixelRenderer::pixels[i][3] / 255)) & 0xff) << 24) +
        //(((static_cast<ulong>(PixelRenderer::pixels[i][1]) * static_cast<ulong>(PixelRenderer::pixels[i][3] / 255)) & 0xff) << 16) +
        //(((static_cast<ulong>(PixelRenderer::pixels[i][2]) * static_cast<ulong>(PixelRenderer::pixels[i][3] / 255)) & 0xff) << 8) +
        //(255u & 0xff);
    }
    PixelRenderer::frameBuffer = CreateBitmap(PixelRenderer::width, PixelRenderer::height, 1, 32, PixelRenderer::renderData.data());

    SelectObject(PixelRenderer::memoryContext, PixelRenderer::frameBuffer);
    BitBlt(PixelRenderer::deviceContext, 0, 0, PixelRenderer::width, PixelRenderer::height, PixelRenderer::memoryContext, 0, 0, SRCCOPY);

    DeleteObject(PixelRenderer::frameBuffer);
}
void PixelRenderer::clear()
{
    for (uint i = 0; i < PixelRenderer::pixels.length(); i++)
    {
        PixelRenderer::pixels[i][0] = PixelRenderer::clearColour[0];
        PixelRenderer::pixels[i][1] = PixelRenderer::clearColour[1];
        PixelRenderer::pixels[i][2] = PixelRenderer::clearColour[2];
        PixelRenderer::pixels[i][3] = 255u;
    }
}