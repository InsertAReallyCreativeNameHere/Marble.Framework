#pragma once

#include "inc.h"

#include <map>
#undef STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace Marble
{
    namespace Typography
    {
        struct GlyphOutline;
        struct GlyphMetrics;

        class coreapi Font final
        {
            stbtt_fontinfo fontInfo;
        public:
            int ascent, descent, lineGap;

            Font(unsigned char* fontData);
            Font(const Font&) = delete;
            Font(Font&&) = delete;

            friend struct Marble::Typography::GlyphOutline;
            friend struct Marble::Typography::GlyphMetrics;
        };

        struct coreapi GlyphOutline final
        {
            Font& font;
            stbtt_vertex* verts;
            int vertsSize;

            GlyphOutline(Font& font, char32_t codepoint);
            GlyphOutline(GlyphOutline&& other);
            ~GlyphOutline();
        };
        struct coreapi GlyphMetrics final
        {
            int advanceWidth, leftSideBearing;

            GlyphMetrics(Font& font, char32_t codepoint);
            GlyphMetrics(GlyphMetrics&& other);
            ~GlyphMetrics();
        };
    }
}