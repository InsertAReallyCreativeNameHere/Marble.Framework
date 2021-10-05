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
    }

    struct __marble_corelib_api Scene final
    {
        inline void* operator new (size_t size);
        inline void operator delete (void* data);

        ~Scene();

        inline const std::string& name();
        size_t index();

        friend class Marble::Internal::CoreEngine;
        friend class Marble::SceneManager;
        friend class Marble::Entity;
    private:
        bool active = false;
        std::list<Entity*> entities;
        std::string sceneName = "Untitled";
    };
    
    class __marble_corelib_api SceneManager final
    {
        struct SceneMemoryChunk
        {
            std::list<SceneMemoryChunk>::iterator it;
            alignas(Scene) char data[sizeof(Scene)];
        };
        static std::list<SceneMemoryChunk> existingScenes;
    public:
        SceneManager() = delete;

        inline static void setSceneActive(Scene* scene)
        {
            scene->active = true;
        }
        inline static void setSceneInactive(Scene* scene)
        {
            scene->active = false;
        }
        inline static void setMainScene(Scene* scene)
        {
            scene->active = true;
            //SceneManager::existingScenes.splice(SceneManager::existingScenes.begin(), SceneManager::existingScenes, scene->it);
        }

        static std::list<Scene*> getScenesByName(const std::string_view& name);

        friend class Marble::Internal::CoreEngine;
        friend struct Marble::Scene;
        friend class Marble::Entity;
    };

    void* Scene::operator new (size_t size)
    {
        SceneManager::existingScenes.emplace_back();
        return SceneManager::existingScenes.back().data;
    }
    void Scene::operator delete (void* data)
    {
        SceneManager::existingScenes.erase(static_cast<Scene*>(data)->it);
    }

    const std::string& Scene::name()
    {
        return this->sceneName;
    }
}