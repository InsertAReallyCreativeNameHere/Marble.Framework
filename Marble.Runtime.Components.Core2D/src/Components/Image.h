#pragma once

#include "inc.h"
#include "Marble.Runtime.Components.Core2D.Exports.h"

#include <robin_hood.h>
#include <Components/PackageFileCore.h>
#include <Core/CoreEngine.h>
#include <Core/PackageManager.h>
#include <Drawing/Core.h>
#include <Objects/Component.h>
#include <Rendering/Renderer.h>
#include <Utility/Property.h>

namespace Marble
{
    namespace Internal
    {
        struct ComponentCoreStaticInit;
    }

    class __marble_componentcore_api Image final : public Internal::Component
    {
        struct __marble_componentcore_api RenderData final
        {
            uint32_t accessCount;
            GL::Texture2DHandle internalTexture;
            PackageSystem::PortableGraphicPackageFile* file;
        };

        static robin_hood::unordered_map<PackageSystem::PortableGraphicPackageFile*, RenderData*> imageTextures;
        RenderData* data = nullptr;

        void setImageFile(PackageSystem::PortableGraphicPackageFile* value);

        void renderOffload();
    public:
        ~Image() override;

        const Property<PackageSystem::PortableGraphicPackageFile*, PackageSystem::PortableGraphicPackageFile*> imageFile
        {{
            [this]() -> PackageSystem::PortableGraphicPackageFile* { return this->data ? this->data->file : nullptr; },
            [this](PackageSystem::PortableGraphicPackageFile* value) { this->setImageFile(value); }
        }};

        friend struct Marble::Internal::ComponentCoreStaticInit;
    };
}