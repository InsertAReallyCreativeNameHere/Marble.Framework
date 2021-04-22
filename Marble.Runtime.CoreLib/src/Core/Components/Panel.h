#pragma once

#include <inc.h>
#include <SDL.h>

#include <Core/Objects/Component.h>
#include <Rendering/Core.h>

namespace Marble
{
    class coreapi Panel final : public Internal::Component
    {
        Internal::Texture2D* data;
        Color _color { 0, 0, 0, 255 };

        void createTexture();
        void destroyTexture();
    public:
        Property<const Color&, const Color&> color;

        Panel();
        ~Panel() override;

        friend class Marble::Internal::CoreEngine;
    };
}