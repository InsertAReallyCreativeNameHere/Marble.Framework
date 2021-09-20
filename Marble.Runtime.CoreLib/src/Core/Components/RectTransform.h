#pragma once

#include <inc.h>
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
        int top, right, bottom, left;

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
        RectTransform();
        ~RectTransform() override;

        RectFloat _rect { 10, 10, -10, -10 };
        RectFloat _anchor { 0, 0, 0, 0 };

        Mathematics::Vector2 _position { 0, 0 };
        float _rotation = 0;
        Mathematics::Vector2 _scale { 1, 1 };

        RectTransform* _parent = nullptr;
        std::list<RectTransform*> _children;
    public:
        Property<const RectFloat&, const RectFloat&> rect;
        Property<const RectFloat&, const RectFloat&> rectAnchor;

        Property<Mathematics::Vector2, Mathematics::Vector2> position;
        Property<float, float> rotation;
        Property<Mathematics::Vector2, Mathematics::Vector2> scale;
        
        Property<Mathematics::Vector2, Mathematics::Vector2> localPosition;
        Property<float, float> localRotation;
        Property<Mathematics::Vector2, Mathematics::Vector2> localScale;

        Property<RectTransform*, RectTransform*> parent;
        inline const std::list<RectTransform*>& children()
        {
            return this->_children;
        }

        friend class Marble::Entity;
    };
}
