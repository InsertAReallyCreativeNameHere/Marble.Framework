#include "Font.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

using namespace Marble::Typography;

GlyphOutline::~GlyphOutline()
{
    STBTT_free(this->verts, nullptr);
}

Font::Font(unsigned char* fontData)
{
    stbtt_InitFont(&this->fontInfo, fontData, 0);
    stbtt_GetFontVMetrics(&this->fontInfo, &this->ascent, &this->descent, &this->lineGap);
}

GlyphOutline Font::getGlyphOutline(int glyphIndex)
{
    GlyphOutline ret;
    ret.vertsSize = stbtt_GetGlyphShape(&this->fontInfo, glyphIndex, &ret.verts);
    return std::move(ret);
}
GlyphMetrics Font::getGlyphMetrics(int glyphIndex)
{
    GlyphMetrics ret;
    stbtt_GetGlyphHMetrics(&this->fontInfo, glyphIndex, &ret.advanceWidth, &ret.leftSideBearing);
    return ret;
}
int Font::getGlyphIndex(char32_t codepoint)
{
    return stbtt_FindGlyphIndex(&this->fontInfo, codepoint);
}