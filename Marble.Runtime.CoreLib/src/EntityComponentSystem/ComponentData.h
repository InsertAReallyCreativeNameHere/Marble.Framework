#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

#include <Objects/Component.h>

namespace Marble
{
    struct ComponentData : public Internal::Component
    {
        virtual ~ComponentData() = 0;
    };
    ComponentData::~ComponentData() = default;
}