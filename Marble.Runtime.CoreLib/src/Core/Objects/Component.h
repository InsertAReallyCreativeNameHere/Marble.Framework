#pragma once

#include <inc.h>

#include <Core/Objects/Object.h>
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
            void (*onDestroy)(Component*);
            
            Entity* attachedEntity;
            RectTransform* attachedRectTransform;
        protected:
            Component();
            virtual ~Component() override = 0;
        public:
            Entity* entity();
            RectTransform* rectTransform();

            friend class Marble::Entity;
            friend class Marble::RectTransform;
            friend class Marble::Internal::CoreEngine;
        };
 }
}

#include <Core/Objects/Entity.h>
#include <Core/Components/RectTransform.h>
#include <Core/CoreEngine.h>
