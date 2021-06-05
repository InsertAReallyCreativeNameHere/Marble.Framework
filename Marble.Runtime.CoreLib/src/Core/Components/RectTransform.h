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

    struct coreapi Rect final
    {
        int top, right, bottom, left;

        Rect(const int& top, const int& right, const int& bottom, const int& left);
    };
    struct coreapi RectFloat final
    {
        float top, right, bottom, left;

        RectFloat(const float& top, const float& right, const float& bottom, const float& left);
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

        Property<const Mathematics::Vector2&, const Mathematics::Vector2&> position;
        Property<float, float> rotation;
        Property<const Mathematics::Vector2&, const Mathematics::Vector2&> scale;
        
        Property<Mathematics::Vector2, Mathematics::Vector2> localPosition;
        Property<float, float> localRotation;
        Property<Mathematics::Vector2, const Mathematics::Vector2&> localScale;

        Property<const RectTransform*, RectTransform*> parent;
        inline const std::list<RectTransform*>& children()
        {
            return this->_children;
        }

        friend class Marble::Entity;
        friend class Marble::Internal::CoreEngine;
    };
}
