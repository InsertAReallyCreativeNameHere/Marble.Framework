#include "Debug.h"

#include <EntityComponentSystem/SceneManagement.h>
#include <Objects/Entity.h>

using namespace Marble;

SpinLock Debug::outputLock;

#if !defined(_WIN32) || WINDOWS_ENABLE_COLOURED_CONSOLE_TEXT
// NB: Don't change this array.
const wchar_t* Debug::ansiCodes[6] =
{
    L"\x1b[0m",
    L"\x1b[1;34m",
    L"\x1b[38;2;255;165;0m",
    L"\x1b[0;32m",
    L"\x1b[38;2;255;255;0m",
    L"\x1b[0;31m"
};
#endif

void Debug::printHierarchy()
{
    using namespace Marble::Internal;

    Debug::outputLock.lock();

    auto printEntity = [&](auto&& printEntity, Entity* entity, int depth) -> void
    {
        for (auto it = entity->childrenFront; it != nullptr; it = it->next)
        {
            for (int i = 0; i < depth; i++)
                std::wcout << L'\t';
            std::wcout << it->name << L'\n';
            printEntity(printEntity, it, depth + 1);
        }
    };

    for (auto _it1 = SceneManager::existingScenes.begin(); _it1 != SceneManager::existingScenes.end(); ++_it1)
    {
        auto it1 = reinterpret_cast<Scene*>(&_it1->data);

        std::wcout << it1->sceneName << L'\n';
        for (auto it2 = it1->front; it2 != nullptr; it2 = it2->next)
        {
            std::wcout << L'\t' << it2->name << L'\n';
            printEntity(printEntity, it2, 2);
        }
    }

    Debug::outputLock.unlock();
}
