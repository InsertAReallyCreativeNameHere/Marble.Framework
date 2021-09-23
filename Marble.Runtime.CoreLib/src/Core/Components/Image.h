#pragma once

#include "inc.h"

#include <robin_hood.h>
#include <Core/Components/PackageFileCore.h>
#include <Core/CoreEngine.h>
#include <Core/Objects/Component.h>
#include <Core/PackageManager.h>
#include <Drawing/Core.h>
#include <Rendering/Core/Renderer.h>
#include <Utility/Property.h>

namespace Marble
{
    namespace Internal
    {
        struct ComponentCoreStaticInit;
    }

    class coreapi Image final : public Internal::Component
    {
        struct coreapi RenderData final
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