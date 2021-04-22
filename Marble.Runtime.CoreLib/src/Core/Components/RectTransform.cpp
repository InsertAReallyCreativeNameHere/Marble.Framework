#include "RectTransform.h"

#include <cmath>

using namespace Marble;
using namespace Marble::Internal;
using namespace Marble::Mathematics;

constexpr static auto rotatePointAround = [](Vector2& point, const Vector2& rotateAround, float angle) -> void
{
    float s = sinf(-angle * float(M_PI) / 180);
    float c = cosf(-angle * float(M_PI) / 180);

    float x = point.x - rotateAround.x;
    float y = point.y - rotateAround.y;

    point.x = x * c - y * s + rotateAround.x;
    point.y = x * s + y * c + rotateAround.y;
};
constexpr static auto rotatePointAroundOrigin = [](Vector2& point, float angle) -> void
{
    float s = sinf(-angle * float(M_PI) / 180);
    float c = cosf(-angle * float(M_PI) / 180);

    float x = point.x;
    float y = point.y;

    point.x = x * c - y * s;
    point.y = x * s + y * c;
};

Rect::Rect(const int& top, const int& right, const int& bottom, const int& left)
: top(top), right(right), bottom(bottom), left(left)
{
}

RectFloat::RectFloat(const float& top, const float& right, const float& bottom, const float& left)
: top(top), right(right), bottom(bottom), left(left)
{
}

RectTransform::RectTransform() :
rect
(
    [this]() -> const RectFloat&
    {
        return this->_rect;
    },
    [this](const RectFloat& value)
    {
        this->_rect = value;
    }
),
rectAnchor
(
    [this]() -> const RectFloat&
    {
        return this->_anchor;
    },
    [this](const RectFloat& value)
    {
        this->_anchor = value;
    }
),
position
(
    [this]() -> const Vector2&
    {
        return this->_position;
    },
    [this](const Vector2& value)
    {
        Vector2 delta = value - this->_position;
        this->_position = value;
        
        auto setChildrenPositionRecursive = [&](auto& recurseFunc, RectTransform* rectTransform) -> void
        {
            for (auto it = rectTransform->_children.begin(); it != rectTransform->_children.end(); ++it)
            {
                (*it)->_position += delta;
                recurseFunc(recurseFunc, *it);
            }
        };
        setChildrenPositionRecursive(setChildrenPositionRecursive, this);
    }
),
rotation
(
    [this]() -> float
    {
        return this->_rotation;
    },
    [this](float value)
    {
        float delta = value - this->_rotation;
        this->_rotation = value;
        
        auto setChildrenRotationRecursive = [&, this](auto& recurseFunc, RectTransform* rectTransform) -> void
        {
            for (auto it = rectTransform->_children.begin(); it != rectTransform->_children.end(); ++it)
            {
                (*it)->_rotation += delta;
                rotatePointAround((*it)->_position, this->_position, delta);
                recurseFunc(recurseFunc, *it);
            }
        };
        setChildrenRotationRecursive(setChildrenRotationRecursive, this);
    }
),
scale
(
    [this]() -> const Vector2&
    {
        return this->_scale;
    },
    [this](const Vector2& value)
    {
        Vector2 delta = value / this->_scale;
        this->_scale = value;
        
        auto setChildrenScaleRecursive = [&, this](auto& recurseFunc, RectTransform* rectTransform) -> void
        {
            for (auto it = rectTransform->_children.begin(); it != rectTransform->_children.end(); ++it)
            {
                (*it)->_scale *= delta;
                (*it)->_position = delta * ((*it)->_position - this->_position) + this->_position;
                recurseFunc(recurseFunc, *it);
            }
        };
        setChildrenScaleRecursive(setChildrenScaleRecursive, this);
    }
),
localPosition
(
    [this]() -> Vector2
    {
        if (this->_parent != nullptr)
        {
            Vector2 locPos = (this->_position - this->_parent->_position) / this->_parent->_scale;
            rotatePointAroundOrigin(locPos, -this->_parent->_rotation);
            return locPos;
        }
        else return this->_position;
    },
    [this](Vector2 value)
    {
        Vector2 delta;
        if (this->_parent != nullptr)
        {
            rotatePointAroundOrigin(value, this->_parent->_rotation);
            delta = this->_parent->_position + this->_parent->_scale * value - this->_position;
        }
        else delta = value - this->_position;
        this->_position += delta;

        auto setChildrenScaleRecursive = [&](auto& recurseFunc, RectTransform* rectTransform) -> void
        {
            for (auto it = rectTransform->_children.begin(); it != rectTransform->_children.end(); ++it)
            {
                (*it)->_position += delta;
                recurseFunc(recurseFunc, *it);
            }
        };
        setChildrenScaleRecursive(setChildrenScaleRecursive, this);
    }
),
localRotation
(
    [this]() -> float
    {
        if (this->_parent != nullptr)
            return this->_rotation - this->_parent->_rotation;
        else return this->_rotation;
    },
    [this](const float& value)
    {
        float delta;
        if (this->_parent != nullptr)
            delta = this->_parent->_rotation + value - this->_rotation;
        else delta = value - this->_rotation;
        this->_rotation += delta;

        auto setChildrenRotationRecursive = [&, this](auto& recurseFunc, RectTransform* rectTransform) -> void
        {
            for (auto it = rectTransform->_children.begin(); it != rectTransform->_children.end(); ++it)
            {
                (*it)->_rotation += delta;
                recurseFunc(recurseFunc, *it);
                rotatePointAround((*it)->_position, this->_position, delta);
            }
        };
        setChildrenRotationRecursive(setChildrenRotationRecursive, this);
    }
),
localScale
(
    [this]() -> Vector2
    {
        if (this->_parent != nullptr)
            return this->_scale / this->_parent->_scale;
        else return this->_scale;
    },
    [this](const Vector2& value)
    {
        Vector2 delta;
        if (this->_parent != nullptr)
            delta = value * this->_parent->_scale / this->_scale;
        else delta = value / this->_scale;
        this->_scale *= delta;

        auto setChildrenScaleRecursive = [&, this](auto& recurseFunc, RectTransform* rectTransform) -> void
        {
            for (auto it = rectTransform->_children.begin(); it != rectTransform->_children.end(); ++it)
            {
                (*it)->_scale *= delta;
                (*it)->_position = delta * ((*it)->_position - this->_position) + this->_position;
                recurseFunc(recurseFunc, *it);
            }
        };
        setChildrenScaleRecursive(setChildrenScaleRecursive, this);
    }
),
parent
(
    [this]() -> const RectTransform*
    {
        return this->_parent;
    },
    [this](RectTransform* value)
    {
        if (this->_parent != nullptr)
        {
            for (auto it = this->_parent->_children.begin(); it != this->_parent->_children.end(); ++it)
            {
                if (*it == this)
                {
                    this->_parent->_children.erase(it);
                    break;
                }
            }
        }
        this->_parent = value;
        if (this->_parent != nullptr)
            this->_parent->_children.push_back(this);
    }
)
{
}
RectTransform::~RectTransform()
{
}