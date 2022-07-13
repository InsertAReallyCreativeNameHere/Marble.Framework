#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

#include <vector>
#include <list>
#include <new>
#include <type_traits>
#include <robin_hood.h>
#include <Mathematics.h>
#include <Core/Debug.h>
#include <Components/RectTransform.h>
#include <Objects/Component.h>
#include <Utility/ManagedArray.h>
#include <Utility/Hash.h>
#include <Utility/TypeInfo.h>

namespace Marble
{
    class Scene;
    class SceneManager;
    class Entity;
    class EntityManager;
    class Debug;
    
    namespace Internal
    {
        class CoreEngine;
    }

    class EntityLevel
    {
        Entity* childrenFront;

        inline EntityLevel(Entity* front) : childrenFront(front)
        {
        }
    public:
        struct EntityIterator
        {
            inline EntityIterator(Entity* entity) : entity(entity)
            {
            }

            inline Entity* operator*() const
            {
                return this->entity;
            }
            inline Entity* operator->() const
            {
                return this->entity;
            }
            inline EntityIterator& operator++();
            inline EntityIterator operator++(int);
        private:
            Entity* entity;
        };

        EntityIterator begin() { return { this->childrenFront }; }
        EntityIterator end() { return { nullptr }; }

        friend class Marble::Entity;
    };
    
    class __marble_corelib_api Entity final : public Internal::Object
    {
        Scene* attachedScene;
        RectTransform* attachedRectTransform;

        Entity* next = nullptr;
        Entity* prev = nullptr;

        Entity* _parent = nullptr;
        Entity* childrenFront = nullptr;
        Entity* childrenBack = nullptr;

        //size_t getIndex();
        //void setIndex(size_t value);

        void insertBefore(Entity* entity);
        void insertAfter(Entity* entity);
        void eraseFromImplicitList();

        void setParent(Entity* value);
    public:
        std::wstring name = L"Untitled";

        Entity();
        Entity(Entity* parent);
        Entity(const Mathematics::Vector2& localPosition, float localRotation, Entity* parent = nullptr);
        Entity(const Mathematics::Vector2& localPosition, float localRotation, const Mathematics::Vector2& scale, Entity* parent = nullptr);
        ~Entity() override;

        inline RectTransform* rectTransform()
        {
            return this->attachedRectTransform;
        }
        
        const Property<Entity*, Entity*> parent
        {{
            [this]() -> Entity* { return this->_parent; },
            [this](Entity* value) { this->setParent(value); }
        }};
        inline EntityLevel children() const
        {
            return { this->childrenFront };
        }

        void moveBefore(Entity* entity);
        void moveAfter(Entity* entity);

        /*Property<size_t, size_t> index
        {{
            [this]() -> size_t { return this->_index; },
            [this](size_t value) { this->setIndex(value); }
        }};*/

        template <typename T>
        inline T* addComponent();
        template <typename T>
        inline T* getComponent();
        template <typename T>
        inline void removeComponent();

        friend class Marble::SceneManager;
        friend class Marble::Scene;
        friend class Marble::EntityManager;
        friend class Marble::EntityLevel::EntityIterator;
        friend class Marble::Internal::Component;
        friend class Marble::RectTransform;
        friend class Marble::Internal::CoreEngine;
        friend class Marble::Debug;
    };

    EntityLevel::EntityIterator& EntityLevel::EntityIterator::operator++()
    {
        this->entity = this->entity->next;
        return *this;
    }
    EntityLevel::EntityIterator EntityLevel::EntityIterator::operator++(int)
    {
        EntityLevel::EntityIterator ret = *this;
        this->entity = this->entity->next;
        return ret;
    }
}
