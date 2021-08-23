#pragma once

#include <inc.h>

#include <Core/Objects/Component.h>

namespace Marble
{
    struct coreapi ComponentData : public Internal::Component
    {
        virtual ~ComponentData() = 0;
    };
}