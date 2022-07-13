#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

#include <initializer_list>
#include <list>
#include <tuple>
#include <robin_hood.h>
#include <EntityComponentSystem/SceneManagement.h>
#include <Objects/internal/Entity.nodep.h>
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
            for
            (
                auto _it1 = SceneManager::existingScenes.begin();
                _it1 != SceneManager::existingScenes.end();
                ++_it1
            )
            {
                Scene* it1 = reinterpret_cast<Scene*>(&_it1->data);
                if (it1->active)
                {
                    for
                    (
                        auto it2 = it1->front;
                        it2 != nullptr;
                        it2 = it2->next
                    )
                    { func(it2); }
                }
            }
        }

        template <typename Func>
        inline static void foreachComponent(const Func&& func)
        {
            EntityManager::foreachEntity([&](Entity* entity)
            {
                for
                (
                    auto it = EntityManager::existingComponents.begin();
                    it != EntityManager::existingComponents.end();
                    ++it
                )
                {
                    auto component = EntityManager::components.find({ entity->attachedScene, entity, it->first });
                    if (component != EntityManager::components.end() && component->second->active)
                        func(component->second);
                }
            });
        }

        template <typename... Ts, typename Func>
        inline static void foreachEntityWithAll(const Func&& func)
        {
            EntityManager::foreachEntity([&](Entity* entity)
            {
                Internal::Component* arr[sizeof...(Ts)] { std::is_same<Ts, RectTransform>::value ? entity->attachedRectTransform : EntityManager::components.find({ entity->attachedScene, entity, __typeid(Ts).qualifiedNameHash() })->second ... };

                bool pred = true;
                for (auto it = arr; it != arr + sizeof...(Ts); ++it)
                    pred = pred && (*it != nullptr);

                size_t i = 0;
                if (pred)
                    func(entity, (Ts*)arr[i++] ...);
            });
        }

        friend class Marble::Entity;
        friend class Marble::Internal::Component;
        friend class Marble::Internal::CoreEngine;
    };
}
