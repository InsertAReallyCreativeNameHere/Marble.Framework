#include "SceneManagement.h"

using namespace Marble;

std::vector<Scene*> SceneManager::existingScenes;

Scene::Scene()
{
    SceneManager::existingScenes.push_back(this);
}
Scene::~Scene()
{
    for (auto it = this->entities.begin(); it != this->entities.end(); ++it)
    {
        (*it)->onDestroy = [](Entity*) {  };
        delete *it;
    }

    for (auto it = SceneManager::existingScenes.begin(); it != SceneManager::existingScenes.end(); ++it)
    {
        if (*it == this)
        {
            SceneManager::existingScenes.erase(it);
            break;
        }
    }
}

size_t Scene::index()
{
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

void SceneManager::setSceneActive(Scene* scene)
{
    scene->active = true;
}
void SceneManager::setSceneInactive(Scene* scene)
{
    scene->active = false;
}
void SceneManager::setMainScene(Scene* scene)
{
    for (auto it = SceneManager::existingScenes.begin(); it != SceneManager::existingScenes.end(); ++it)
    {
        if (*it == scene)
        {
            SceneManager::existingScenes.erase(it);
            break;
        }
    }
    scene->active = true;
    SceneManager::existingScenes.insert(SceneManager::existingScenes.begin(), scene);
}