#include "SceneManagement.h"

using namespace Marble;

std::list<Scene*> SceneManager::existingScenes;

static struct Init {
    Init()
    {
        SceneManager::setSceneActive(new Scene);
    }
} init;

Scene::~Scene()
{
    ProfileFunction();
    
    for (auto it = this->entities.begin(); it != this->entities.end(); ++it)
    {
        (*it)->eraseIteratorOnDestroy = false;
        delete *it;
    }
    this->entities.clear();

    if (this->eraseIteratorOnDestroy)
        SceneManager::existingScenes.erase(this->it);
}

size_t Scene::index()
{
    ProfileFunction();
    
    size_t i = 0;
    for (auto it = SceneManager::existingScenes.begin(); it != SceneManager::existingScenes.end(); ++it)
    {
        if (!(*it)->active)
            continue;
        if (*it == this)
            return i;
        i++;
    }
    Debug::LogError("Could not determine the index of the scene! Is the scene active?");
    return SIZE_MAX;
}
