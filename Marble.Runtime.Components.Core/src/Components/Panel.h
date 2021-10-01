#pragma once

#include "inc.h"
#include "Marble.Runtime.Components.Core.Exports.h"

#include <Drawing/Core.h>
#include <Objects/Component.h>
#include <Utility/Property.h>

namespace Marble
{
    namespace Internal
    {
        struct ComponentCoreStaticInit;
    }

    class __marble_componentcore_api Panel final : public Internal::Component
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