#include "RectTransform.h"

#include <cmath>
#include <Objects/Entity.h>
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
    ProfileFunction();
    
    Vector2 delta = value - this->_position;
    this->_position = value;

    auto setChildrenPositionRecursive = [&](auto& recurseFunc, Entity* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedRectTransform->_position += delta;
            recurseFunc(recurseFunc, it);
        }
    };
    setChildrenPositionRecursive(setChildrenPositionRecursive, this->attachedEntity);
}
void RectTransform::setRotation(float value)
{
    ProfileFunction();

    float delta = value - this->_rotation;
    this->_rotation = value;

    auto setChildrenRotationRecursive = [&, this](auto& recurseFunc, Entity* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedRectTransform->_rotation += delta;
            rotatePointAround(it->attachedRectTransform->_position, this->_position, delta);
            recurseFunc(recurseFunc, it);
        }
    };
    setChildrenRotationRecursive(setChildrenRotationRecursive, this->attachedEntity);
}
void RectTransform::setScale(Vector2 value)
{
    ProfileFunction();

    Vector2 delta = value / this->_scale;
    this->_scale = value;

    auto setChildrenScaleRecursive = [&, this](auto& recurseFunc, Entity* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedRectTransform->_scale *= delta;
            it->attachedRectTransform->_position = delta * (it->attachedRectTransform->_position - this->_position) + this->_position;
            recurseFunc(recurseFunc, it);
        }
    };
    setChildrenScaleRecursive(setChildrenScaleRecursive, this->attachedEntity);
}

Vector2 RectTransform::getLocalPosition() const
{
    Entity* parent = this->attachedEntity->_parent;
    if (parent)
    {
        Vector2 locPos = (this->_position - parent->attachedRectTransform->_position) / parent->attachedRectTransform->_scale;
        rotatePointAroundOrigin(locPos, -parent->attachedRectTransform->_rotation);
        return locPos;
    }
    else return this->_position;
}
void RectTransform::setLocalPosition(Vector2 value)
{
    ProfileFunction();

    Vector2 delta;
    Entity* parent = this->attachedEntity->_parent;
    if (parent)
    {
        rotatePointAroundOrigin(value, parent->attachedRectTransform->_rotation);
        delta = parent->attachedRectTransform->_position + parent->attachedRectTransform->_scale * value - this->_position;
    }
    else delta = value - this->_position;
    this->_position += delta;

    auto setChildrenScaleRecursive = [&](auto&& recurseFunc, Entity* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedRectTransform->_position += delta;
            recurseFunc(recurseFunc, it);
        }
    };
    setChildrenScaleRecursive(setChildrenScaleRecursive, this->attachedEntity);
}
float RectTransform::getLocalRotation() const
{
    Entity* parent = this->attachedEntity->_parent;
    return parent ? this->_rotation - parent->attachedRectTransform->_rotation : this->_rotation;
}
void RectTransform::setLocalRotation(float value)
{
    ProfileFunction();

    Entity* parent = this->attachedEntity->_parent;

    float delta;
    if (parent)
        delta = parent->attachedRectTransform->_rotation + value - this->_rotation;
    else delta = value - this->_rotation;
    this->_rotation += delta;

    auto setChildrenRotationRecursive = [&, this](auto& recurseFunc, Entity* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedRectTransform->_rotation += delta;
            recurseFunc(recurseFunc, it);
            rotatePointAround(it->attachedRectTransform->_position, this->_position, delta);
        }
    };
    setChildrenRotationRecursive(setChildrenRotationRecursive, this->attachedEntity);
}
Vector2 RectTransform::getLocalScale() const
{
    Entity* parent = this->attachedEntity->_parent;
    return parent ? this->_scale / parent->attachedRectTransform->_scale : this->_scale;
}
void RectTransform::setLocalScale(Vector2 value)
{
    ProfileFunction();

    Entity* parent = this->attachedEntity->_parent;

    Vector2 delta;
    if (parent)
        delta = value * parent->attachedRectTransform->_scale / this->_scale;
    else delta = value / this->_scale;
    this->_scale *= delta;

    auto setChildrenScaleRecursive = [&, this](auto& recurseFunc, Entity* entity) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            it->attachedRectTransform->_scale *= delta;
            it->attachedRectTransform->_position = delta * (it->attachedRectTransform->_position - this->_position) + this->_position;
            recurseFunc(recurseFunc, it);
        }
    };
    setChildrenScaleRecursive(setChildrenScaleRecursive, this->attachedEntity);
}

bool RectTransform::queryPointIn(const Vector2& point)
{
    float rL = this->_rect.left * this->_scale.x;
    float rB = this->_rect.bottom * this->_scale.y;

    Vector2 vA(this->_rect.right * this->_scale.x, rB);
    rotatePointAroundOrigin(vA, this->_rotation);
    Vector2 vB(rL, rB);
    rotatePointAroundOrigin(vB, this->_rotation);
    Vector2 vC(rL, this->_rect.top * this->_scale.y);
    rotatePointAroundOrigin(vC, this->_rotation);

    Vector2 vAB = vB - vA;
    Vector2 vBC = vC - vB;

    vA += this->_position;
    vB += this->_position;
    vC += this->_position;

    Vector2 vAM = point - vA;
    Vector2 vBM = point - vB;

    float dABAM = vAB.dot(vAM);
    float dBCBM = vBC.dot(vBM);

    return 0 <= dABAM && dABAM <= vAB.dot(vAB) &&
    0 <= dBCBM && dBCBM <= vBC.dot(vBC);
}
