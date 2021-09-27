#pragma once

#include "inc.h"

#include <list>
#include <Core/Objects/Entity.h>
 
namespace Marble
{
    class SceneManager;
    class Entity;

    namespace Internal
    {
        class CoreEngine;
    }

    struct Scene;

    class coreapi SceneManager final
    {
        static std::list<Scene*> existingScenes;
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

    struct coreapi Scene final
    {
        inline Scene()
        {
            SceneManager::existingScenes.push_back(this);
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
        std::list<Scene*>::iterator it;
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
        SceneManager::existingScenes.erase(scene->it);
        scene->active = true;
        SceneManager::existingScenes.insert(SceneManager::existingScenes.begin(), scene);
    }
}