#include "RectTransform.h"

#include <cmath>
#include <Mathematics.h>

using namespace Marble;
using namespace Marble::Internal;
using namespace Marble::Mathematics;

constexpr static auto rotatePointAround = [](Vector2& point, const Vector2& rotateAround, float angle) -> void
{
    float s = sinf(-angle * float(piF) / 180);
    float c = cosf(-angle * float(piF) / 180);

    float x = point.x - rotateAround.x;
    float y = point.y - rotateAround.y;

    point.x = x * c - y * s + rotateAround.x;
    point.y = x * s + y * c + rotateAround.y;
};
constexpr static auto rotatePointAroundOrigin = [](Vector2& point, float angle) -> void
{
    float s = sinf(-angle * float(piF) / 180);
    float c = cosf(-angle * float(piF) / 180);

    float x = point.x;
    float y = point.y;

    point.x = x * c - y * s;
    point.y = x * s + y * c;
};

void RectTransform::setPosition(Vector2 value)
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
void RectTransform::setRotation(float value)
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
void RectTransform::setScale(Vector2 value)
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

Vector2 RectTransform::getLocalPosition() const
{
    if (this->_parent)
    {
        Vector2 locPos = (this->_position - this->_parent->_position) / this->_parent->_scale;
        rotatePointAroundOrigin(locPos, -this->_parent->_rotation);
        return locPos;
    }
    else return this->_position;
}
void RectTransform::setLocalPosition(Vector2 value)
{
    Vector2 delta;
    if (this->_parent)
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
void RectTransform::setLocalRotation(float value)
{
    float delta;
    if (this->_parent)
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
void RectTransform::setLocalScale(Vector2 value)
{
    Vector2 delta;
    if (this->_parent)
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

void RectTransform::setParent(RectTransform* value)
{
    if (this->_parent)
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
    if (this->_parent)
        this->_parent->_children.push_back(this);
}
