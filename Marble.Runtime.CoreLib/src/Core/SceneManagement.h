#pragma once

#include <inc.h>

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

    struct coreapi Scene final
    {
        Scene();
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

    class coreapi SceneManager final
    {
        static std::list<Scene*> existingScenes;
    public:
        SceneManager() = delete;

        static void setSceneActive(Scene* scene);
        static void setSceneInactive(Scene* scene);
        static void setMainScene(Scene* scene);

        static std::list<Scene*> getScenesByName(const std::string_view& name);

        friend class Marble::Internal::CoreEngine;
        friend struct Marble::Scene;
        friend class Marble::Entity;
    };
}