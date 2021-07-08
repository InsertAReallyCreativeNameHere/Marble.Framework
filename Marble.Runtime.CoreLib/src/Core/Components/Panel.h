#pragma once

#include <inc.h>

#include <Core/Objects/Component.h>
#include <Rendering/Core.h>

namespace Marble
{
    namespace Internal
    {
        class CoreEngine;
    }

    class coreapi Panel final : public Internal::Component
    {
        Color _color { 0, 0, 0, 255 };
    public:
        Property<const Color&, const Color&> color;

        Panel();
        ~Panel() override;

        friend class Marble::Internal::CoreEngine;
    };
}