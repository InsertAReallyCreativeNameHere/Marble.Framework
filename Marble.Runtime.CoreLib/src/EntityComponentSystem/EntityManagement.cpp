#include "EntityManagement.h"

#include <EntityComponentSystem/SceneManagement.h>

using namespace Marble;

std::list<Internal::EntityChunk> EntityManager::entities;
std::list<Internal::EntityChunk>::iterator EntityManager::lastAllocChunk;
size_t EntityManager::lastAllocIndex;

robin_hood::unordered_map<uint64_t, void*> componentMaps;

void* EntityManager::allocateEntity()
{
    for (auto it = EntityManager::entities.begin(); it != EntityManager::entities.end(); ++it)
    {
        if (it->fragments.length() > 0)
            return &it->entities[it->fragments.popBack()].mem;
        else if (it->next == Config_EntitiesPerChunk)
            goto PushNewChunk;
        else return &it->entities[it->next++].mem;
    }

    PushNewChunk:
    EntityManager::entities.emplace_back();
    auto it = --EntityManager::entities.end();
    EntityManager::lastAllocChunk = it;
    EntityManager::lastAllocIndex = 0;
    return &it->entities[0].mem;
}
void EntityManager::destroyEntity(Entity* entity)
{
    auto& chunk = entity->chunk;
    auto& frags = chunk->fragments;
    frags.emplaceBack(entity->index);
    entity->~Entity();
    if (frags.length() == chunk->next)
        EntityManager::entities.erase(chunk);
}
