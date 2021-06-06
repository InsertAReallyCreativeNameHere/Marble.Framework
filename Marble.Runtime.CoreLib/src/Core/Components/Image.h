#pragma once

#include <inc.h>

#include <Core/CoreEngine.h>
#include <Core/Objects/Component.h>
#include <Core/PackageManager.h>
#include <Rendering/Core/Texture.h>
#include <Utility/Property.h>
#include <Rendering/Core.h>
#include <SDL.h>

namespace Marble
{
    namespace Internal
    {
        class CoreEngine;
    }

    class coreapi Image final : public Internal::Component
    {
        struct RenderData
        {
            GL::Texture2D* internalTexture = nullptr;
        };
        RenderData* data;
    public:
        Image();
        ~Image();

        Property<PackageSystem::PortableGraphicPackageFile*, PackageSystem::PortableGraphicPackageFile*> imageFile;

        friend class Marble::Internal::CoreEngine;
    };
}