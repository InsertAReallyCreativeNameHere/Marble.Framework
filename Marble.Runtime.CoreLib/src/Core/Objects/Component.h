#pragma once

#include "inc.h"

#include <list>
#include <Core/Objects/Object.h>

namespace Marble
{
    class Entity;
    class RectTransform;

    namespace Internal
    {
        class CoreEngine;
        
        class coreapi Component : public Object
        {
            struct coreapi Reflection final {
                uint64_t typeID;
            } reflection;

            std::list<Component*>::iterator it;
            bool eraseIteratorOnDestroy = true;
            
            Entity* attachedEntity = nullptr;
            RectTransform* attachedRectTransform = nullptr;
        protected:
            virtual ~Component();
        public:
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
        };
    }
}
