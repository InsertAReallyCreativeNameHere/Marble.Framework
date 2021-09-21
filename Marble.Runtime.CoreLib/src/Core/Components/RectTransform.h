#pragma once

#include "inc.h"
#include <Core/Objects/Component.h>
#include <Utility/Property.h>
#include <Mathematics.h>

namespace Marble
{
    namespace Internal
    {
        class CoreEngine;
    }

    struct Rect
    {
        int32_t top, right, bottom, left;

        constexpr Rect(int32_t top, int32_t right, int32_t bottom, int32_t left) :
        top(top), right(right), bottom(bottom), left(left)
        {
        }
    };
    struct RectFloat
    {
        float top, right, bottom, left;

        constexpr RectFloat(float top, float right, float bottom, float left) :
        top(top), right(right), bottom(bottom), left(left)
        {
        }
    };

    class coreapi RectTransform final : public Internal::Component
    {
        inline ~RectTransform() override = default;

        RectFloat _rect { 10, 10, -10, -10 };
        RectFloat _anchor { 0, 0, 0, 0 };

        Mathematics::Vector2 _position { 0, 0 };
        float _rotation = 0;
        Mathematics::Vector2 _scale { 1, 1 };

        RectTransform* _parent = nullptr;
        std::list<RectTransform*> _children;

        void setPosition(Mathematics::Vector2 value);
        void setRotation(float value);
        void setScale(Mathematics::Vector2 value);

        Mathematics::Vector2 getLocalPosition() const;
        void setLocalPosition(Mathematics::Vector2 value);
        void setLocalRotation(float value);
        void setLocalScale(Mathematics::Vector2 scale);

        void setParent(RectTransform* value);
    public:
        const Property<const RectFloat&, const RectFloat&> rect
        {{
            [this]() -> const RectFloat& { return this->_rect; },
            [this](const RectFloat& value) { this->_rect = value; }
        }};
        const Property<const RectFloat&, const RectFloat&> rectAnchor
        {{
            [this]() -> const RectFloat& { return this->_anchor; },
            [this](const RectFloat& value) { this->_anchor = value; }
        }};

        const Property<Mathematics::Vector2, Mathematics::Vector2> position
        {{
            [this]() -> Mathematics::Vector2 { return this->_position; },
            [this](Mathematics::Vector2 value) { this->setPosition(value); }
        }};
        const Property<float, float> rotation
        {{
            [this]() -> float { return this->_rotation; },
            [this](float value) { this->setRotation(value); }
        }};
        const Property<Mathematics::Vector2, Mathematics::Vector2> scale
        {{
            [this]() -> Mathematics::Vector2 { return this->_scale; },
            [this](Mathematics::Vector2 value) { this->setScale(value); }
        }};
        
        const Property<Mathematics::Vector2, Mathematics::Vector2> localPosition
        {{
            [this]() -> Mathematics::Vector2 { return this->getLocalPosition(); },
            [this](Mathematics::Vector2 value) { this->setLocalPosition(value); }
        }};
        const Property<float, float> localRotation
        {{
            [this]() -> float { return this->_parent ? this->_rotation - this->_parent->_rotation : this->_rotation; },
            [this](float value) { this->setLocalRotation(value); }
        }};
        const Property<Mathematics::Vector2, Mathematics::Vector2> localScale
        {{
            [this]() -> Mathematics::Vector2 { return this->_parent ? this->_scale / this->_parent->_scale : this->_scale; },
            [this](Mathematics::Vector2 value) { this->setLocalScale(value); }
        }};

        const Property<RectTransform*, RectTransform*> parent
        {{
            [this]() -> RectTransform* { return this->_parent; },
            [this](RectTransform* value) { this->setParent(value); }
        }};
        inline const std::list<RectTransform*>& children() const
        {
            return this->_children;
        }

        friend class Marble::Entity;
    };
}
