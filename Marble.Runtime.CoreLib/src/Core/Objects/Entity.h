#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <inc.h>

#include <vector>
#include <list>
#include <new>
#include <type_traits>
#include <cstring>
#include <ctti/nameof.hpp>
#include <Core/Debug.h>
#include <Core/Components/RectTransform.h>
#include <Core/Objects/Component.h>
#include <Utility/ManagedArray.h>
#include <Utility/Hash.h>
#include <Mathematics.h>

namespace Marble
{
    class Entities;
    class Components;
    
    class Scene;
    class SceneManager;

    namespace Internal
    {
        class CoreEngine;
    }

    class coreapi Entity final : public Internal::Object
    {
        std::list<Entity*>::iterator it;
        bool eraseIteratorOnDestroy = true;

        Scene* attachedScene;
        RectTransform* attachedRectTransform;

        std::list<Internal::Component*> components {  };
        
        void removeComponentInternal(Internal::Component* component);
    public:
        Entity();
        Entity(RectTransform* parent);
        Entity(const Mathematics::Vector2& localPosition, const float& localRotation, RectTransform* parent);
        Entity(const Mathematics::Vector2& localPosition, const float& localRotation, const Mathematics::Vector2& scale, RectTransform* parent);
        ~Entity() override;

        Entity* entity();
        RectTransform* rectTransform();

        template <typename T>
        T* addComponent()
        {
            static_assert(!std::is_same<T, RectTransform>::value, "Cannot add component to Entity. You cannot add RectTransform to an Entity, it is a default component.");
            static_assert(std::is_base_of<Internal::Component, T>::value, "Cannot add component to Entity. Typename \"T\" is not derived from type \"Component\"");

            T* ret = new T();
            ret->attachedEntity = this;
            ret->attachedRectTransform = this->attachedRectTransform;
            ret->reflection.typeID = strhash(ctti::nameof<T>().begin());
            this->components.push_back(ret);
            ret->it = --this->components.end();
            return ret;
        }
        template <typename T>
        T* getFirstComponent()
        {
            if constexpr (std::is_same<T, RectTransform>::value)
            {
                return this->attachedRectTransform;
            }
            else
            {
                static_assert(std::is_base_of<Internal::Component, T>::value, "Cannot get component from Entity. Typename \"T\" is not derived from type \"Component\"");

                for (auto it = this->components.begin(); it != this->components.end(); ++it)
                {
                    if ((*it)->reflection.typeID == strhash(ctti::nameof<T>().begin()))
                    {
                        return static_cast<T*>(*it);
                    }
                }

                Debug::LogWarn("No component of type \"", ctti::nameof<T>().begin(), "\" could be found on this Entity!");
                return nullptr;
            }
        }
        template <typename T>
        std::vector<T*> getAllComponents()
        {
            if constexpr (std::is_same<T, RectTransform>::value)
            {
                return std::vector<T*> { this->attachedRectTransform };
            }
            else
            {
                static_assert(std::is_base_of<Internal::Component, T>::value, "Cannot get component from Entity. Typename \"T\" is not derived from type \"Component\"");

                std::vector<T*> components;
                for (auto it = this->components.begin(); it != this->components.end(); ++it)
                {
                    if (it->second == strhash(ctti::nameof<T>().begin()))
                    {
                        components.push_back(static_cast<T*>(it->first));
                    }
                }

                if (components.empty())
                    Debug::LogWarn("No component of type \"", ctti::nameof<T>().begin(), "\" could be found on this Entity!");
                return components;
            }
        }
        template <typename T>
        void removeFirstComponent()
        {
            static_assert(!std::is_same<T, RectTransform>::value, "Cannot remove component from Entity. You cannot remove RectTransform from an Entity, it is a default component.");
            static_assert(std::is_base_of<Internal::Component, T>::value, "Cannot get component from Entity. Typename \"T\" is not derived from type \"Component\"");
            
            for (auto it = this->components.begin(); it != this->components.end(); ++it)
            {
                if (it->second == strhash(ctti::nameof<T>().begin()))
                {
                    it->first->onDestroy = [](Internal::Component*) {  };
                    delete it->first;
                    this->components.erase(it);
                    return;
                }
            }
            
            Debug::LogWarn("No identical component could be found on this Entity to be removed!");
        }
        template <typename T>
        void removeAllComponents()
        {
            static_assert(!std::is_same<T, RectTransform>::value, "Cannot remove component from Entity. You cannot remove RectTransform from an Entity, it is a default component.");

            bool throwWarn = true;
            for (auto it = this->components.begin(); it != this->components.end();)
            {
                if (it->second == strhash(ctti::nameof<T>().begin()))
                {
                    it->first->onDestroy = [](Internal::Component*) {  };
                    delete it->first;
                    it = this->components.erase(it);
                    throwWarn = false;
                }
                else ++it;
            }

            if (throwWarn)
                Debug::LogWarn("No identical component could be found on this Entity to be removed!");
        }

        friend class Marble::Internal::Component;
        friend class Marble::Entities;
        friend class Marble::Components;
        friend class Marble::SceneManager;
        friend class Marble::Scene;
        friend class Marble::Internal::CoreEngine;
    };
}

#endif