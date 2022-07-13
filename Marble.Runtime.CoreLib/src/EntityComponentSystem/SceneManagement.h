#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

#include <list>
#include <string>
 
namespace Marble
{
    class SceneManager;
    class EntityManager;
    class Entity;
    class Debug;

    namespace Internal
    {
        class CoreEngine;
        struct SceneMemoryChunk;
    }

    struct __marble_corelib_api Scene final
    {
        inline const std::string& name()
        {
            return this->sceneName;
        }
        size_t index();

        friend class Marble::Internal::CoreEngine;
        friend class Marble::SceneManager;
        friend class Marble::EntityManager;
        friend class Marble::Entity;
        friend class Marble::Debug;
    private:
        Entity* front = nullptr;
        Entity* back = nullptr;
        
        inline Scene();
        ~Scene();

        std::list<Internal::SceneMemoryChunk>::iterator it;
        bool active = false;
        std::string sceneName = "Untitled";
    };

    namespace Internal
    {
        struct SceneMemoryChunk
        {
            alignas(Scene) char data[sizeof(Scene)];
        };
    }
    
    class __marble_corelib_api SceneManager final
    {
        static std::list<Internal::SceneMemoryChunk> existingScenes;
    public:
        SceneManager() = delete;

        inline static Scene* createScene()
        {
            SceneManager::existingScenes.emplace_back();
            return new(SceneManager::existingScenes.back().data) Scene;
        }
        inline static void destroyScene(Scene* scene)
        {
            auto it = scene->it;
            scene->~Scene();
            SceneManager::existingScenes.erase(it);
        }

        inline static void setSceneActive(Scene* scene, bool active)
        {
            scene->active = active;
        }
        inline static void setMainScene(Scene* scene)
        {
            scene->active = true;
            SceneManager::existingScenes.splice(SceneManager::existingScenes.begin(), SceneManager::existingScenes, scene->it);
        }

        inline static Scene* getSceneByIndex(size_t index)
        {
            if (index < SceneManager::existingScenes.size())
                return reinterpret_cast<Scene*>(&std::next(SceneManager::existingScenes.begin(), index)->data);
            else return nullptr;
        }

        friend class Marble::Internal::CoreEngine;
        friend struct Marble::Scene;
        friend class Marble::EntityManager;
        friend class Marble::Entity;
        friend class Marble::Debug;
    };

    Scene::Scene() : it(--SceneManager::existingScenes.end())
    {
    }
}