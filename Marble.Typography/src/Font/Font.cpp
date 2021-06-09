#include "Font.h"

#include <limits>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

using namespace Marble::Typography;

FontAtlas::FontAtlas(uint32_t width, uint32_t height) :
data({ new uint8_t[width * height * 4], width, height })
{
}
FontAtlas::~FontAtlas()
{
    delete[] this->data.rgbaData;
}

const FontAtlasPixelData& FontAtlas::getPixelData() const
{
    return this->data;
}
const std::map<char, FontGlyph>& FontAtlas::getGlyphMap() const
{
    return this->glyphs;
}

bool FontAtlas::pushGlyph(char c, uint8_t* rgbaData, uint32_t width, uint32_t height)
{
    for (uint32_t j = 0; j < this->data.height - height + 1;)
    {
        uint32_t yIncrement = std::numeric_limits<uint32_t>::max();
        for (uint32_t i = 0; i < this->data.width - width + 1;)
        {
            uint32_t xIncrement = 1; // Just in case my code is bad and stuff doesn't work.
            for (auto it = this->glyphs.begin(); it != this->glyphs.end(); ++it)
            {
                if
                (
                    i < it->second.x + it->second.width && it->second.x < i + width &&
                    j < it->second.y + it->second.height && it->second.y < j + height
                )
                {
                    uint32_t yDelta = it->second.y - j + it->second.height; // Optimise to not search each pixel coord.
                    if (yDelta < yIncrement)
                        yIncrement = yDelta;
                    xIncrement = it->second.x - i + it->second.width; // Optimise to not search each pixel coord.
                    goto Continue;
                }
            }

            // Runs when available rectangle for glyph is found.
            {
                auto& glyph = this->glyphs[c];
                glyph.x = i;
                glyph.y = j;
                glyph.width = width;
                glyph.height = height;

                for (uint32_t _i = 0; _i < width; _i++)
                    for (uint32_t _j = 0; _j < height; _j++)
                        this->data.rgbaData[(_j + j) * this->data.width * 4 + (_i + i) * 4] = rgbaData[_j * width * 4 + _i * 4];

                return true;
            }

            // Execution jumps to here when available rectangle for glyph is not found.
            Continue:
            i += xIncrement;
        }
        j += yIncrement;
    }
    return false;
}
void FontAtlas::removeGlyph(char c)
{
    this->glyphs.erase(c);
}

TrueTypeFontAtlas::TrueTypeFontAtlas(uint8_t* ttfData, void (*freeTtfData)(uint8_t*), uint32_t width, uint32_t height) :
atlas(width, height), freeTtfData(freeTtfData)
{
    stbtt_InitFont(&this->ttInfo, ttfData, 0);
}
TrueTypeFontAtlas::~TrueTypeFontAtlas()
{
    this->freeTtfData(this->ttInfo.data);
}

const FontAtlas& TrueTypeFontAtlas::getAtlas() const
{
    return this->atlas;
}

bool TrueTypeFontAtlas::loadCharsErasingFirstIfOutOfMemory(const char* chars, float scale)
{
    uint32_t len = strlen(chars);

    int width = 0, height = 0;
    int xLower, xHigher, yLower, yHigher;
    int w, h;
    for (uint32_t i = 0; i < len; i++)
    {
        stbtt_GetCodepointBitmapBox(&this->ttInfo, chars[i], scale, scale, &xLower, &yLower, &xHigher, &yHigher);
        w = xHigher - xLower;
        h = yHigher - yLower;
        if (width < w)
            width = w;
        if (height < h)
            height = h;
    }

    if (width > this->atlas.data.width || height > this->atlas.data.height)
        return false;

    int xoff, yoff;
    uint8_t* buffer = new uint8_t[width * height * 4];
    for (uint32_t i = 0; i < len; i++)
    {
        stbtt_GetCodepointBitmapBox(&this->ttInfo, wchar_t(chars[i]), scale, scale, &xLower, &yLower, &xHigher, &yHigher);
        w = xHigher - xLower;
        h = yHigher - yLower;
        stbtt_MakeCodepointBitmap(&this->ttInfo, buffer + width * height * 4 - w * h, w, h, w, scale, scale, wchar_t(chars[i]));

        for (uint32_t _i = 0; _i < w * h; _i++)
        {
            switch (buffer[width * height * 4 - w * h + _i])
            {
            case 0x0u:
                buffer[_i * 4] = 0;
                buffer[_i * 4 + 1] = 0;
                buffer[_i * 4 + 2] = 0;
                buffer[_i * 4 + 3] = 0;
                break;
            default:
                buffer[_i * 4] = buffer[width * height * 4 - w * h + _i];
                buffer[_i * 4 + 1] = buffer[width * height * 4 - w * h + _i];
                buffer[_i * 4 + 2] = buffer[width * height * 4 - w * h + _i];
                buffer[_i * 4 + 3] = 0xffu;
            }
        }

        auto it = this->atlas.glyphs.find(chars[i]);
        if (it == this->atlas.glyphs.end())
        {
            while (!this->atlas.pushGlyph(chars[i], buffer, w, h))
                this->atlas.glyphs.erase(this->atlas.glyphs.begin());
        }
    }
    delete[] buffer;

    return true;
}
void TrueTypeFontAtlas::loadCharErasingFirstIfOutOfMemory(char c, float scale)
{
}
void TrueTypeFontAtlas::loadCharsIgnoreIfOutOfMemory(const char* chars, float scale)
{
}
void TrueTypeFontAtlas::loadCharIgnoreIfOutOfMemory(char c, float scale)
{
}