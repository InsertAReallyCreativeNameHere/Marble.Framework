#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

namespace Marble
{
    namespace Internal
    {
        class Object
        {
        public:
            inline virtual ~Object() = 0;
        };

        Object::~Object()
        {
        }
    }
}
