#pragma once

#include <inc.h>

#include <Core/PackageManager.h>
#include <Core/Objects/Component.h>
#include <Extras/Property.h>
#include <Rendering/Core.h>
#include <SDL.h>

namespace Marble
{
    namespace Internal
    {
        class CoreEngine;
        class Renderer;
    }

    class coreapi Image final : public Internal::Component
    {
        static std::unordered_map<PackageSystem::PortableGraphicPackageFile*, Internal::Texture2D> textures;

        Internal::Texture2D* texture = nullptr;
    public:
        Image();

        Property<PackageSystem::PortableGraphicPackageFile*, PackageSystem::PortableGraphicPackageFile*> imageFile;

        friend class Marble::Internal::Renderer;
        friend class Marble::Internal::CoreEngine;
    };
}