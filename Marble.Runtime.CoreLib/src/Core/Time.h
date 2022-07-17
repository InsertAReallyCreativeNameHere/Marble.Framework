#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

namespace Marble
{
    namespace Internal
    {
        class CoreEngine;
    }

    class __marble_corelib_api Time final
    {
        static float frameDeltaTime;
    public:
        static float timeScale;
        
        inline static float deltaTime()
        {
            return Time::frameDeltaTime;
        }

        friend class Marble::Internal::CoreEngine;
    };
}