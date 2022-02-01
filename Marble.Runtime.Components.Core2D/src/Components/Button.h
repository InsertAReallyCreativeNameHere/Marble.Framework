#pragma once

#include "inc.h"
#include "Marble.Runtime.Components.Core2D.Exports.h"

#include <Drawing/Core.h>
#include <Objects/Component.h>
#include <Rendering/Renderer.h>

namespace Marble
{
    namespace Internal
    {
        struct ComponentCoreStaticInit;
    }

    enum class ButtonState
    {
        Idle = 0b00,
        Highlighted = 0b01,
        Pressed = 0b11
    };

    class __marble_componentcore_api ColorHighlightButton final : public Internal::Component
    {
        ButtonState buttonState;

        void renderOffload();
    public:
        Color idleColor { 0, 0, 0, 0 };
        Color highlightColor { 125, 125, 125, 125 };
        Color pressedColor { 255, 255, 255, 255 };

        inline ButtonState state()
        {
            return this->buttonState;
        }

        friend struct Marble::Internal::ComponentCoreStaticInit;
    };
}