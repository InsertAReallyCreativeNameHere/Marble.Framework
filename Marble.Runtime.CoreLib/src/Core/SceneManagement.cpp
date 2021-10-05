#include "SceneManagement.h"

using namespace Marble;
using namespace Marble::Internal;

std::list<Internal::SceneMemoryChunk> SceneManager::existingScenes;

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
}

size_t Scene::index()
{
    ProfileFunction();
    
    size_t i = 0;
    for (auto it = SceneManager::existingScenes.begin(); it != SceneManager::existingScenes.end(); ++it)
    {
        if (reinterpret_cast<Scene*>(&it->data) == this)
            return i;
        ++i;
    }
    Debug::LogError("Could not determine the index of the scene! Is the scene active?");
    return SIZE_MAX;
}
