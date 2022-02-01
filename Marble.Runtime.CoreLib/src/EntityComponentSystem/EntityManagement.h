#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"
#include "Config.h"
static_assert
(
    Config_EntitiesPerChunk > 0 && Config_EntitiesPerChunk <= SIZE_MAX,
    "The value of Config_EntitiesPerChunk is required to be larger than 0 and less than or equal to SIZE_MAX."
);

#include <list>
#include <robin_hood.h>
#include <Objects/Entity.h>
#include <Utility/StaticQueue.h>

namespace Marble
{
    namespace Internal
    {
        struct EntityMemory
        {
            alignas(Entity) char mem[sizeof(Entity)];

            inline Entity* ent()
            { return reinterpret_cast<Entity*>(&this->mem); }
        };
        struct EntityChunk
        {
            EntityMemory entities[Config_EntitiesPerChunk];
            size_t next = 1;
            StaticQueue<size_t, Config_EntitiesPerChunk> fragments;
        };
    }

    class Scene;
    class Entity;

    class __marble_corelib_api EntityManager final
    {
        static std::list<Internal::EntityChunk> entities;
        static std::list<Internal::EntityChunk>::iterator lastAllocChunk;
        static size_t lastAllocIndex;

        static robin_hood::unordered_map<uint64_t, void*> componentMaps;

        static void* allocateEntity();
        static void destroyEntity(Entity* entity);
    public:
        EntityManager() = delete;

        friend class Marble::Scene;
        friend class Marble::Entity;
    };
}