#pragma once

#include <inc.h>

#include <Core/CoreEngine.h>
#include <Core/Objects/Component.h>
#include <Core/PackageManager.h>
#include <Rendering/Core.h>
#include <Rendering/Core/Renderer.h>
#include <Utility/Property.h>
#include <SDL.h>

namespace Marble
{
    namespace Internal
    {
        class CoreEngine;
    }

    class coreapi Image final : public Internal::Component
    {
        struct coreapi RenderData final
        {
            uint32_t accessCount;
            GL::Texture2DHandle internalTexture;
            PackageSystem::PortableGraphicPackageFile* file;
        };

        static std::unordered_map<PackageSystem::PortableGraphicPackageFile*, RenderData*> imageTextures;
        RenderData* data;
    public:
        Image();
        ~Image();

        Property<PackageSystem::PortableGraphicPackageFile*, PackageSystem::PortableGraphicPackageFile*> imageFile;

        friend class Marble::Internal::CoreEngine;
    };
}