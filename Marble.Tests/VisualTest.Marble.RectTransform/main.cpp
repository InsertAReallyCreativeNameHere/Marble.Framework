#include <cmath>
#include <Components/Panel.h>
#include <Core/Time.h>
#include <EntityComponentSystem/EngineEvent.h>
#include <Marble/Entry.h>
#include <Objects/Entity.h>

using namespace Marble;

Entity* entity;
float t = 0.0f;

#define main __main(int argc, char* argv[]); int main(int argc, char* argv[]) { __main(argc, argv); handleInitializeAndExit(argc, argv); } int __main

int main(int argc, char* argv[])
{
    EngineEvent::OnInitialize += []
    {
        entity = new Entity();
        entity->addComponent<Panel>()->color = { 0, 255, 255, 255 };
        entity->rectTransform()->rect = { 50, 50, -50, -50 };
        
        EntityManager::foreachEntityWithAll<Panel, RectTransform>([](Entity* ent, Panel*, RectTransform*) { Debug::LogInfo(ent->name); });
    };
    EngineEvent::OnTick += []
    {
        entity->rectTransform()->rotation = t;
        entity->rectTransform()->position = { cosf(t) * 10, sinf(t) * 10 };
        t += 360000.f * Time::deltaTime();
    };

    return 0;
}