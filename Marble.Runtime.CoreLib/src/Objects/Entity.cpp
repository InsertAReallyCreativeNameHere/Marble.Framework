#include "Entity.h"

#include <EntityComponentSystem/SceneManagement.h>

using namespace Marble;
using namespace Marble::Internal;
using namespace Marble::Mathematics;

#define thisRect this->attachedRectTransform

Entity::Entity()
{
    ProfileFunction();

    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    this->attachedRectTransform->eraseIteratorOnDestroy = false;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    this->_index = mainScene->entities.size();
    mainScene->entities.push_back(this);
    this->it = --mainScene->entities.end();
    this->attachedScene = mainScene;
}
Entity::Entity(RectTransform* parent)
{
    ProfileFunction();

    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    this->attachedRectTransform->parent = parent;
    this->attachedRectTransform->eraseIteratorOnDestroy = false;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    this->_index = mainScene->entities.size();
    mainScene->entities.push_back(this);
    this->it = --mainScene->entities.end();
    this->attachedScene = mainScene;
}
Entity::Entity(const Vector2& localPosition, float localRotation, RectTransform* parent = nullptr)
{
    ProfileFunction();

    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    thisRect->parent = parent;
    thisRect->localPosition = localPosition;
    thisRect->localRotation = localRotation;
    thisRect->eraseIteratorOnDestroy = false;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    this->_index = mainScene->entities.size();
    mainScene->entities.push_back(this);
    this->it = --mainScene->entities.end();
    this->attachedScene = mainScene;
}
Entity::Entity(const Vector2& localPosition, float localRotation, const Vector2& scale, RectTransform* parent = nullptr)
{
    ProfileFunction();

    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    thisRect->parent = parent;
    thisRect->localPosition = localPosition;
    thisRect->localRotation = localRotation;
    thisRect->scale = scale;
    thisRect->eraseIteratorOnDestroy = false;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    this->_index = mainScene->entities.size();
    mainScene->entities.push_back(this);
    this->it = --mainScene->entities.end();
    this->attachedScene = mainScene;
}
Entity::~Entity()
{
    ProfileFunction();
    
    for (auto it = this->components.begin(); it != this->components.end(); ++it)
    {
        (*it)->eraseIteratorOnDestroy = false;
        delete *it;
    }
    delete this->attachedRectTransform;

    if (this->eraseIteratorOnDestroy)
        this->attachedScene->entities.erase(this->it);
}

void Entity::setIndex(size_t value)
{
    if (value < this->attachedScene->entities.size())
    {
        this->attachedScene->entities.splice
        (
            std::next(this->attachedScene->entities.begin(), value),
            this->attachedScene->entities, this->it
        );
        this->_index = value;
        auto it = std::next(this->it);
        while (it != this->attachedScene->entities.end())
        {
            ++(*it)->_index;
            ++it;
        }
    }
}
