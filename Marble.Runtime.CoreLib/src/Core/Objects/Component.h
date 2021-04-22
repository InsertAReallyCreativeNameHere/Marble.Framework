#pragma once

#include <inc.h>

#include <Core/Objects/Object.h>
#include <Extras/Lock.h>

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
        protected:
            Component();
            virtual ~Component() override = 0;
        public:
            Entity* attachedEntity;
            RectTransform* attachedRectTransform;

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
