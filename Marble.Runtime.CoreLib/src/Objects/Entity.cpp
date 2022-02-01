#include "Entity.h"

#include <EntityComponentSystem/EntityManagement.h>
#include <EntityComponentSystem/SceneManagement.h>

using namespace Marble;
using namespace Marble::Internal;
using namespace Marble::Mathematics;

void* Entity::operator new(size_t)
{
    return EntityManager::allocateEntity();
}
void Entity::operator delete (Entity* entity, std::destroying_delete_t)
{
    EntityManager::destroyEntity(entity);
}

#define thisRect this->attachedRectTransform

Entity::Entity()
{
    ProfileFunction();

    this->chunk = EntityManager::lastAllocChunk;
    this->index = EntityManager::lastAllocIndex;

    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    this->attachedRectTransform->eraseIteratorOnDestroy = false;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    mainScene->entities.push_back(this);
    this->attachedScene = mainScene;
}
Entity::Entity(RectTransform* parent)
{
    ProfileFunction();

    this->chunk = EntityManager::lastAllocChunk;
    this->index = EntityManager::lastAllocIndex;

    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    this->attachedRectTransform->parent = parent;
    this->attachedRectTransform->eraseIteratorOnDestroy = false;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    mainScene->entities.push_back(this);
    this->attachedScene = mainScene;
}
Entity::Entity(const Vector2& localPosition, float localRotation, RectTransform* parent = nullptr)
{
    ProfileFunction();

    this->chunk = EntityManager::lastAllocChunk;
    this->index = EntityManager::lastAllocIndex;

    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    thisRect->parent = parent;
    thisRect->localPosition = localPosition;
    thisRect->localRotation = localRotation;
    thisRect->eraseIteratorOnDestroy = false;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    mainScene->entities.push_back(this);
    this->attachedScene = mainScene;
}
Entity::Entity(const Vector2& localPosition, float localRotation, const Vector2& scale, RectTransform* parent = nullptr)
{
    ProfileFunction();

    this->chunk = EntityManager::lastAllocChunk;
    this->index = EntityManager::lastAllocIndex;

    this->attachedRectTransform = new RectTransform();
    this->attachedRectTransform->attachedEntity = this;
    this->attachedRectTransform->attachedRectTransform = this->attachedRectTransform;
    thisRect->parent = parent;
    thisRect->localPosition = localPosition;
    thisRect->localRotation = localRotation;
    thisRect->scale = scale;
    thisRect->eraseIteratorOnDestroy = false;

    Scene* mainScene = reinterpret_cast<Scene*>(&SceneManager::existingScenes.front().data);
    mainScene->entities.push_back(this);
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
}
