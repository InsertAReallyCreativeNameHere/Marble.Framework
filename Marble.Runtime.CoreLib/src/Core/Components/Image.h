#pragma once

#include <inc.h>

#include <robin_hood.h>
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
        RenderData* data;

        void renderOffload();
    public:
        Image();
        ~Image();

        Property<PackageSystem::PortableGraphicPackageFile*, PackageSystem::PortableGraphicPackageFile*> imageFile;

        friend class Marble::Internal::ComponentCoreStaticInit;
    };
}