#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

#include <list>
#include <Objects/Object.h>
#include <Utility/MemoryChunk.h>
#include <Utility/MinAllocList.h>
#include <Utility/Property.h>

namespace Marble
{
    class Entity;
    class RectTransform;
    class Debug;

    namespace Internal
    {
        class CoreEngine;
        
        class __marble_corelib_api Component : public Object
        {
            struct Reflection {
                uint64_t typeID;
            } reflection;

            Entity* attachedEntity = nullptr;
            RectTransform* attachedRectTransform = nullptr;
        protected:
            ~Component() override;
        public:
            bool active = true;

            inline uint64_t typeIndex()
            {
                return this->reflection.typeID;
            }

            inline Entity* entity()
            {
                return this->attachedEntity;
            }
            inline RectTransform* rectTransform()
            {
                return this->attachedRectTransform;
            }

            friend class Marble::Entity;
            friend class Marble::RectTransform;
            friend class Marble::Internal::CoreEngine;
            friend class Marble::Debug;
        };
    }
}
