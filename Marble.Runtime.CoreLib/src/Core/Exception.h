#pragma once

#include <inc.h>

#include <exception>

namespace Marble
{
    struct coreapi NoMainSceneException final : std::exception
    {
        const char* what();
    };
}