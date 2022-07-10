#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

#include <list>
#include <tuple>
#include <robin_hood.h>
#include <EntityComponentSystem/SceneManagement.h>
#include <Utility/MinAllocList.h>
#include <Utility/TupleHash.h>

namespace Marble
{
    class Scene;
    class Entity;

    namespace Internal
    {
        class Component;
        class CoreEngine;
    }

    class __marble_corelib_api EntityManager final
    {
        static robin_hood::unordered_map<uint64_t, uint64_t> existingComponents;
        static robin_hood::unordered_map<std::tuple<Scene*, Entity*, uint64_t>, Internal::Component*> components;
    public:
        EntityManager() = delete;

        template <typename Func>
        inline static void foreachEntity(const Func&& func)
        {
            /*for (auto it1 = SceneManager::existingScenes.begin(); it1 != SceneManager::existingScenes.end(); ++it1)
            {
                for (auto it2 = reinterpret_cast<Scene*>(it1->data)->first; it2 != nullptr; it2 = it2->next)
                {
                    func(it2);
                }
            }*/
        }

        friend class Marble::Entity;
        friend class Marble::Internal::Component;
        friend class Marble::Internal::CoreEngine;
    };
}
