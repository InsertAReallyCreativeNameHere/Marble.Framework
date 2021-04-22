#include "Entity.h"

#include <Core/SceneManagement.h>
#include <Core/Exception.h>

using namespace Marble;
using namespace Marble::Internal;
using namespace Marble::Mathematics;

#define thisRect this->attachedRectTransform

Entity::Entity() :
onDestroy
(
    [](Entity* _this)
    {
        for
        (
            auto it = _this->attachedScene->entities.begin();
            it != _this->attachedScene->entities.end();
            ++it
        )
        {
            if (*it == _this)
            {
                _this->attachedScene->entities.erase(it);
                break;
            }
        }
    }
)
{
    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;

    auto mainScene = SceneManager::existingScenes.front();
    mainScene->entities.push_back(this);
    this->attachedScene = mainScene;
}
Entity::Entity(RectTransform* parent) :
onDestroy
(
    [](Entity* _this)
    {
        for
        (
            auto it = _this->attachedScene->entities.begin();
            it != _this->attachedScene->entities.end();
            ++it
        )
        {
            if (*it == _this)
            {
                _this->attachedScene->entities.erase(it);
                break;
            }
        }
    }
)
{
    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    this->attachedRectTransform->parent = parent;

    auto mainScene = SceneManager::existingScenes.front();
    mainScene->entities.push_back(this);
    this->attachedScene = mainScene;
}
Entity::Entity(const Vector2& localPosition, const float& localRotation, RectTransform* parent = nullptr) :
onDestroy
(
    [](Entity* _this)
    {
        for
        (
            auto it = _this->attachedScene->entities.begin();
            it != _this->attachedScene->entities.end();
            ++it
        )
        {
            if (*it == _this)
            {
                _this->attachedScene->entities.erase(it);
                break;
            }
        }
    }
)
{
    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    thisRect->parent = parent;
    thisRect->localPosition = localPosition;
    thisRect->localRotation = localRotation;

    auto mainScene = SceneManager::existingScenes.front();
    mainScene->entities.push_back(this);
    this->attachedScene = mainScene;
}
Entity::Entity(const Vector2& localPosition, const float& localRotation, const Vector2& scale, RectTransform* parent = nullptr) :
onDestroy
(
    [](Entity* _this)
    {
        for
        (
            auto it = _this->attachedScene->entities.begin();
            it != _this->attachedScene->entities.end();
            ++it
        )
        {
            if (*it == _this)
            {
                _this->attachedScene->entities.erase(it);
                break;
            }
        }
    }
)
{
    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    thisRect->parent = parent;
    thisRect->localPosition = localPosition;
    thisRect->localRotation = localRotation;
    thisRect->scale = scale;

    auto mainScene = SceneManager::existingScenes.front();
    mainScene->entities.push_back(this);
    this->attachedScene = mainScene;
}
Entity::~Entity()
{
    for (auto it = this->components.begin(); it != this->components.end(); ++it)
    {
        it->first->onDestroy = [](Component*) {  };
        delete it->first;
    }

    this->onDestroy(this);
}

Entity* Entity::entity()
{
    return this;
}
RectTransform* Entity::rectTransform()
{
    return this->attachedRectTransform;
}