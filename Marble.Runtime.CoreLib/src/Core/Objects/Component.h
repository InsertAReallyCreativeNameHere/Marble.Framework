#pragma once

#include <inc.h>

#include <Core/Objects/Object.h>
#include <list>
#include <Utility/Lock.h>

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
            
            Entity* attachedEntity;
            RectTransform* attachedRectTransform;
        protected:
            Component();
            virtual ~Component() = 0;
        public:
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

#include <Core/Objects/Entity.h>
#include <Core/Components/RectTransform.h>
#include <Core/CoreEngine.h>
