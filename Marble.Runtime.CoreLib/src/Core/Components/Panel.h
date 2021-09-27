#pragma once

#include "inc.h"

#include <Core/Objects/Component.h>
#include <Drawing/Core.h>
#include <Utility/Property.h>

namespace Marble
{
    namespace Internal
    {
        struct ComponentCoreStaticInit;
    }

    class coreapi Panel final : public Internal::Component
    {
        Color _color { 0, 0, 0, 255 };

        void renderOffload();
    public:
        const Property<const Color&, const Color&> color
        {{
            [this]() -> const Color& { return this->_color; },
            [this](const Color& value) { this->_color = value; }
        }};

        inline ~Panel() override = default;

        friend class Marble::Internal::ComponentCoreStaticInit;
    };
}