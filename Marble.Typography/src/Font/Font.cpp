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

GlyphOutline Font::getGlyphOutline(int glyph)
{
    GlyphOutline ret;
    ret.vertsSize = stbtt_GetGlyphShape(&this->fontInfo, glyph, &ret.verts);
    return std::move(ret);
}
GlyphOutline::GlyphOutline()
{
}
GlyphOutline::GlyphOutline(GlyphOutline&& other) : verts(other.verts), vertsSize(other.vertsSize)
{
    other.verts = nullptr;
    other.vertsSize = 0;
}
GlyphOutline::~GlyphOutline()
{
    STBTT_free(this->verts, nullptr);
}

GlyphMetrics Font::getGlyphMetrics(int glyph)
{
    GlyphMetrics ret;
    stbtt_GetGlyphHMetrics(&this->fontInfo, glyph, &ret.advanceWidth, &ret.leftSideBearing);
    return ret;
}
GlyphMetrics::GlyphMetrics()
{
}

int Font::getGlyphIndex(char32_t codepoint)
{
    return stbtt_FindGlyphIndex(&this->fontInfo, codepoint);
}