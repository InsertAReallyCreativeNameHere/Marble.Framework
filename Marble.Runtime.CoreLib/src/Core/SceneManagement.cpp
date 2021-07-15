#include "SceneManagement.h"

using namespace Marble;

std::list<Scene*> SceneManager::existingScenes;

static struct Initializer {
    Initializer()
    {
        new Scene;
    }
} init;

Scene::Scene()
{
    SceneManager::existingScenes.push_back(this);
    this->it = --SceneManager::existingScenes.end();
}
Scene::~Scene()
{
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
    SceneManager::existingScenes.erase(scene->it);
    scene->active = true;
    SceneManager::existingScenes.insert(SceneManager::existingScenes.begin(), scene);
}