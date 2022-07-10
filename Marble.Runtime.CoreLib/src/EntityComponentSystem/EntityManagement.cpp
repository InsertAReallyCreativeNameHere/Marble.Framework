#include "EntityManagement.h"

#include <Objects/Entity.h>

using namespace Marble;

robin_hood::unordered_map<uint64_t, uint64_t> EntityManager::existingComponents;
robin_hood::unordered_map<std::tuple<Scene*, Entity*, uint64_t>, Internal::Component*> EntityManager::components;
