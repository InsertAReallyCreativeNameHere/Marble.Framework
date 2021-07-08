#pragma once

#include "inc.h"

#include <Core/Objects/Component.h>
#include <Core/PackageManager.h>
#include <Font/Font.h>
#include <map>
#include <Rendering/Core/Renderer.h>

namespace Marble
{
    namespace Internal
    {
        class CoreEngine;
    }

    class coreapi Text final : public Internal::Component
    {
        struct coreapi CharacterRenderData final
        {
            uint32_t accessCount;
            GL::PolygonHandle polygon;
        };
        struct coreapi RenderData final
        {
            uint32_t accessCount;
            std::map<char32_t, CharacterRenderData*> characters;
            PackageSystem::TrueTypeFontPackageFile* file;
        };

        static std::unordered_map<PackageSystem::TrueTypeFontPackageFile*, RenderData*> textFonts;
        RenderData* data;

        std::u32string _text;
    public:
        Text();
        ~Text();

        Property<PackageSystem::TrueTypeFontPackageFile*, PackageSystem::TrueTypeFontPackageFile*> font;
        Property<const std::u32string&, std::u32string> text;
        uint32_t fontSize; // In pixels.

        friend class Marble::Internal::CoreEngine;
    };
}