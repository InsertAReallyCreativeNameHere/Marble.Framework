#pragma once

#include "inc.h"

#include <map>
#undef STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace Marble
{
    namespace Typography
    {
        typedef struct {
            uint32_t x, y, width, height;
        } FontGlyph;
        typedef struct {
            uint8_t* rgbaData;
            uint32_t width, height;
        } FontAtlasPixelData;

        class TrueTypeFontAtlas;

        class coreapi FontAtlas final
        {
            FontAtlasPixelData data;
            std::map<char, FontGlyph> glyphs;
        public:
            FontAtlas(uint32_t width, uint32_t height);
            ~FontAtlas();

            const FontAtlasPixelData& getPixelData() const;
            const std::map<char, FontGlyph>& getGlyphMap() const;

            // This is very slow, only pushGlyph once ideally.
            bool pushGlyph(char c, uint8_t* rgbaData, uint32_t width, uint32_t height);
            void removeGlyph(char c);

            friend class Marble::Typography::TrueTypeFontAtlas;
        };

        class coreapi TrueTypeFontAtlas final
        {
            FontAtlas atlas;

            stbtt_fontinfo ttInfo;
            void (*freeTtfData)(uint8_t*);
        public:
            TrueTypeFontAtlas(uint8_t* ttfData, void (*freeTtfData)(uint8_t*), uint32_t width, uint32_t height);
            ~TrueTypeFontAtlas();

            const FontAtlas& getAtlas() const;

            bool loadCharsErasingFirstIfOutOfMemory(const char* chars, float scale);
            void loadCharErasingFirstIfOutOfMemory(char c, float scale);
            void loadCharsIgnoreIfOutOfMemory(const char* chars, float scale);
            void loadCharIgnoreIfOutOfMemory(char c, float scale);
        };
    }
}