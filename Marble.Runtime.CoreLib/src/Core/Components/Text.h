#pragma once

#include "inc.h"

#include <robin_hood.h>
#include <stb_truetype.h>
#include <Core/Objects/Component.h>
#include <Core/PackageManager.h>
#include <Font/Font.h>
#include <Rendering/Core/Renderer.h>
#include <Utility/Property.h>

namespace Marble
{
    namespace Internal
    {
        struct ComponentCoreStaticInit;
    }

    enum class TextAlign : uint8_t
    {
        Left,
        Center,
        Right,
        Justify
    };

    struct FontSize
    {
        FontSize() = delete;

        enum
        {
            Default = 11,
            RecalculateForRectSize = UINT32_MAX
        };
    };

    class coreapi Text final : public Internal::Component
    {
        struct CharacterData
        {
            int glyphIndex;
            Typography::GlyphMetrics metrics;
        };
        struct CharacterRenderData
        {
            uint32_t accessCount;
            GL::PolygonHandle polygon;
        };
        struct RenderData
        {
            uint32_t accessCount;
            robin_hood::unordered_map<int, CharacterRenderData*> characters;
            PackageSystem::TrueTypeFontPackageFile* file;

            inline void trackCharacters(const std::vector<CharacterData>& text);
            inline void untrackCharacters(const std::vector<CharacterData>& text);
        };

        static robin_hood::unordered_map<PackageSystem::TrueTypeFontPackageFile*, RenderData*> textFonts;
        RenderData* data;

        std::u32string _text;
        std::vector<CharacterData> textData;
        uint32_t _fontSize;

        void renderOffload();
    public:
        Text();
        ~Text();

        Property<PackageSystem::TrueTypeFontPackageFile*, PackageSystem::TrueTypeFontPackageFile*> font;
        Property<const std::u32string&, std::u32string> text;
        Property<uint32_t, uint32_t> fontSize; // In pixels.
        TextAlign horizontalAlign;
        TextAlign verticalAlign;

        friend class Marble::Internal::ComponentCoreStaticInit;
    };
}