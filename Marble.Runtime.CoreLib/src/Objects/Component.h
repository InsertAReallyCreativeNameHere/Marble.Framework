#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

#include <list>
#include <Objects/Object.h>
#include <Utility/Property.h>

namespace Marble
{
    class Entity;
    class RectTransform;

    namespace Internal
    {
        class CoreEngine;
        
        class __marble_corelib_api Component : public Object
        {
            struct __marble_corelib_api Reflection final {
                uint64_t typeID;
            } reflection;

            std::list<Component*>::iterator it;
            bool eraseIteratorOnDestroy = true;

            Entity* attachedEntity = nullptr;
            RectTransform* attachedRectTransform = nullptr;

            size_t _index;
            void setIndex(size_t value);
        protected:
            virtual ~Component();
        public:
            bool active = true;

            Property<size_t, size_t> index
            {{
                [this]() -> size_t { return this->_index; },
                [this](size_t value) { this->setIndex(value); }
            }};

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
