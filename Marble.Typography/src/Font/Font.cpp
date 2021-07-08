#include "Font.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

using namespace Marble::Typography;

template <typename Char>
constexpr size_t __strlen(const Char* str)
{
    size_t len = 0;
    while (str[len++] != 0);
    return len;
}

Font::Font(unsigned char* fontData)
{
    stbtt_InitFont(&this->fontInfo, fontData, 0);
    stbtt_GetFontVMetrics(&this->fontInfo, &this->ascent, &this->descent, &this->lineGap);
}

GlyphOutline::GlyphOutline(Font& font, char32_t codepoint) : font(font), verts(nullptr)
{
    this->vertsSize = stbtt_GetCodepointShape(&font.fontInfo, codepoint, &this->verts);
}
GlyphOutline::GlyphOutline(GlyphOutline&& other) : font(other.font), verts(other.verts), vertsSize(other.vertsSize)
{
    other.verts = nullptr;
    other.vertsSize = 0;
}
GlyphOutline::~GlyphOutline()
{
    stbtt_FreeShape(&this->font.fontInfo, this->verts);
}

GlyphMetrics::GlyphMetrics(Font& font, char32_t codepoint)
{
    stbtt_GetCodepointHMetrics(&font.fontInfo, codepoint, &this->advanceWidth, &this->leftSideBearing);
}
GlyphMetrics::GlyphMetrics(GlyphMetrics&& other) : advanceWidth(other.advanceWidth), leftSideBearing(other.leftSideBearing)
{
    other.advanceWidth = 0;
    other.leftSideBearing = 0;
}
GlyphMetrics::~GlyphMetrics()
{
}