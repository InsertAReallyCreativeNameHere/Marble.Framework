#pragma once

#include "inc.h"
#include "Marble.Runtime.Components.Core.Exports.h"

#include <robin_hood.h>
#include <Components/PackageFileCore.h>
#include <Core/PackageManager.h>
#include <Font/Font.h>
#include <Objects/Component.h>
#include <Rendering/Core/Renderer.h>
#include <Utility/Property.h>

namespace Marble
{
    namespace Internal
    {
        struct ComponentCoreStaticInit;
    }

    enum class TextAlign : uint_fast8_t
    {
        Minor,
        Center,
        Major,
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

    class __marble_componentcore_api Text final : public Internal::Component
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

            void trackCharacters(const std::vector<CharacterData>& text);
            void untrackCharacters(const std::vector<CharacterData>& text);
        };

        static robin_hood::unordered_map<PackageSystem::TrueTypeFontPackageFile*, RenderData*> textFonts;
        RenderData* data = nullptr;

        inline static float getAlignOffsetJustify(float endDiff, float mul)
        {
            return endDiff * mul;
        }
        inline static float getAlignOffsetMajor(float endDiff, float)
        {
            return endDiff;
        }
        inline static float getAlignOffsetCenter(float endDiff, float)
        {
            return endDiff / 2;
        }
        inline static float getAlignOffsetMinor(float, float)
        {
            return 0.0f;
        }

        std::u32string _text;
        std::vector<CharacterData> textData;
        TextAlign _horizontalAlign = TextAlign::Minor;
        TextAlign _verticalAlign = TextAlign::Minor;
        float (*getHorizontalAlignOffset)(float, float) = getAlignOffsetMinor;
        float (*getVerticalAlignOffset)(float, float) = getAlignOffsetMinor;
        uint32_t _fontSize = 11;

        void setFontFile(PackageSystem::TrueTypeFontPackageFile* value);
        void setText(std::u32string value);
        void setHorizontalAlign(TextAlign value);
        void setVerticalAlign(TextAlign value);
        void setFontSize(uint32_t value);

        void renderOffload();
    public:
        ~Text() override;

        const Property<PackageSystem::TrueTypeFontPackageFile*, PackageSystem::TrueTypeFontPackageFile*> fontFile
        {{
            [this]() -> PackageSystem::TrueTypeFontPackageFile* { return this->data ? this->data->file : nullptr; },
            [this](PackageSystem::TrueTypeFontPackageFile* value) { this->setFontFile(value); }
        }};
        const Property<const std::u32string&, std::u32string> text
        {{
            [this]() -> const std::u32string& { return this->_text; },
            [this](std::u32string value) { this->setText(value); }
        }};
        const Property<uint32_t, uint32_t> fontSize
        {{
            [&]() -> uint32_t { return this->_fontSize; },
            [&, this](uint32_t value) { this->setFontSize(value); }
        }};
        const Property<TextAlign, TextAlign> horizontalAlign
        {{
            [&]() -> TextAlign { return this->_horizontalAlign; },
            [&, this](TextAlign value) { this->setHorizontalAlign(value); }
        }};
        const Property<TextAlign, TextAlign> verticalAlign
        {{
            [&]() -> TextAlign { return this->_verticalAlign; },
            [&, this](TextAlign value) { this->setVerticalAlign(value); }
        }};

        friend class Marble::Internal::ComponentCoreStaticInit;
    };
}