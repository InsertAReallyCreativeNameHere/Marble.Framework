#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

#include <robin_hood.h>

namespace Marble
{
    class __marble_corelib_api EntityManager final
    {
        static std::list<std::array<Entity, 4096>>> entities;
        static robin_hood::unordered_map<uint64_t, void*> componentMaps;
    public:
        EntityManager() = delete;


    };
}