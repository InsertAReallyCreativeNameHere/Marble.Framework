#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

#include <list>
#include <Objects/Entity.h>
 
namespace Marble
{
    class SceneManager;
    class Entity;

    namespace Internal
    {
        class CoreEngine;

        struct SceneMemoryChunk
        {
            // FIXME: This + the static_assert at the bottom is kind of a band-aid solution.
            alignas(Scene) char data[72];
        };
    }

    struct Scene;

    class __marble_corelib_api SceneManager final
    {
        static std::list<Internal::SceneMemoryChunk> existingScenes;
    public:
        SceneManager() = delete;

        inline static void setSceneActive(Scene* scene);
        inline static void setSceneInactive(Scene* scene);
        inline static void setMainScene(Scene* scene);

        static std::list<Scene*> getScenesByName(const std::string_view& name);

        friend class Marble::Internal::CoreEngine;
        friend struct Marble::Scene;
        friend class Marble::Entity;
    };

    struct __marble_corelib_api Scene final
    {
        inline void* operator new (size_t size)
        {
            SceneManager::existingScenes.emplace_back();
            return SceneManager::existingScenes.back().data;
        }
        inline void operator delete (void* data)
        {
            Scene* scene = static_cast<Scene*>(data);
            if (scene->eraseIteratorOnDestroy)
                SceneManager::existingScenes.erase(scene->it);
        }

        inline Scene()
        {
            this->it = --SceneManager::existingScenes.end();
        }
        ~Scene();

        inline const std::string& name()
        {
            return this->sceneName;
        }
        size_t index();

        friend class Marble::Internal::CoreEngine;
        friend class Marble::SceneManager;
        friend class Marble::Entity;
    private:
        std::list<Internal::SceneMemoryChunk>::iterator it;
        bool eraseIteratorOnDestroy = true;

        bool active = false;
        std::list<Entity*> entities;
        std::string sceneName = "Untitled";
    };
    
    inline void SceneManager::setSceneActive(Scene* scene)
    {
        scene->active = true;
    }
    inline void SceneManager::setSceneInactive(Scene* scene)
    {
        scene->active = false;
    }
    inline void SceneManager::setMainScene(Scene* scene)
    {
        scene->active = true;
        SceneManager::existingScenes.splice(SceneManager::existingScenes.begin(), SceneManager::existingScenes, scene->it);
    }

    static_assert(sizeof(Scene) == 72, "The developer is not very smart and needs to change the size of SceneMemoryChunk.");
}