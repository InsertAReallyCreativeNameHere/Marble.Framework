#include "Entity.h"

#include <Core/SceneManagement.h>

using namespace Marble;
using namespace Marble::Internal;
using namespace Marble::Mathematics;

#define thisRect this->attachedRectTransform

Entity::Entity()
{
    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    this->attachedRectTransform->eraseIteratorOnDestroy = false;

    auto mainScene = SceneManager::existingScenes.front();
    mainScene->entities.push_back(this);
    this->it = --mainScene->entities.end();
    this->attachedScene = mainScene;
}
Entity::Entity(RectTransform* parent)
{
    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    this->attachedRectTransform->parent = parent;
    this->attachedRectTransform->eraseIteratorOnDestroy = false;

    auto mainScene = SceneManager::existingScenes.front();
    mainScene->entities.push_back(this);
    this->it = --mainScene->entities.end();
    this->attachedScene = mainScene;
}
Entity::Entity(const Vector2& localPosition, const float& localRotation, RectTransform* parent = nullptr)
{
    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    thisRect->parent = parent;
    thisRect->localPosition = localPosition;
    thisRect->localRotation = localRotation;
    thisRect->eraseIteratorOnDestroy = false;

    auto mainScene = SceneManager::existingScenes.front();
    mainScene->entities.push_back(this);
    this->it = --mainScene->entities.end();
    this->attachedScene = mainScene;
}
Entity::Entity(const Vector2& localPosition, const float& localRotation, const Vector2& scale, RectTransform* parent = nullptr)
{
    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    thisRect->parent = parent;
    thisRect->localPosition = localPosition;
    thisRect->localRotation = localRotation;
    thisRect->scale = scale;
    thisRect->eraseIteratorOnDestroy = false;

    auto mainScene = SceneManager::existingScenes.front();
    mainScene->entities.push_back(this);
    this->it = --mainScene->entities.end();
    this->attachedScene = mainScene;
}
Entity::~Entity()
{
    for (auto it = this->components.begin(); it != this->components.end(); ++it)
    {
        (*it)->eraseIteratorOnDestroy = false;
        delete *it;
    }
    delete this->attachedRectTransform;

    if (this->eraseIteratorOnDestroy)
        this->attachedScene->entities.erase(this->it);
}

Entity* Entity::entity()
{
    return this;
}
RectTransform* Entity::rectTransform()
{
    return this->attachedRectTransform;
}