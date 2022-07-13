#pragma once

#include <EntityComponentSystem/EntityManagement.h>

namespace Marble
{
    template <typename T>
    T* Entity::addComponent()
    {
        static_assert(!std::is_same<T, RectTransform>::value, "Cannot add component to Entity. You cannot add RectTransform to an Entity, it is a default component.");
        static_assert(std::is_base_of<Internal::Component, T>::value, "Cannot add component to Entity. Typename \"T\" is not derived from type \"Component\"");

        if (!EntityManager::components.contains({ this->attachedScene, this, __typeid(T).qualifiedNameHash() }))
        {
            auto it = EntityManager::existingComponents.find(__typeid(T).qualifiedNameHash());
            if (it != EntityManager::existingComponents.end())
                ++it->second;
            else EntityManager::existingComponents.emplace(__typeid(T).qualifiedNameHash(), 1);

            T* ret = new T();
            EntityManager::components.emplace(std::make_tuple(this->attachedScene, this, __typeid(T).qualifiedNameHash()), ret);
            ret->attachedEntity = this;
            ret->attachedRectTransform = this->attachedRectTransform;
            ret->reflection.typeID = __typeid(T).qualifiedNameHash();

            return ret;
        }
        else
        {
            Debug::LogError("Entity already has this type of component!");
            return nullptr;
        }
    }
    template <typename T>
    T* Entity::getComponent()
    {
        if constexpr (std::is_same<T, RectTransform>::value)
            return this->attachedRectTransform;
        else
        {
            static_assert(std::is_base_of<Internal::Component, T>::value, "Cannot get component from Entity. Typename \"T\" is not derived from type \"Component\"");

            auto it = EntityManager::components.find({ this->attachedScene, this, __typeid(T).qualifiedNameHash() });
            if (it != EntityManager::components.end())
                return it;

            Debug::LogWarn("No component of type \"", __typeid(T).qualifiedName(), "\" could be found on this Entity!");
            return nullptr;
        }
    }
    template <typename T>
    void Entity::removeComponent()
    {
        static_assert(!std::is_same<T, RectTransform>::value, "Cannot remove component from Entity. You cannot remove RectTransform from an Entity, it is a default component.");
        static_assert(std::is_base_of<Internal::Component, T>::value, "Cannot get component from Entity. Typename \"T\" is not derived from type \"Component\"");
        
        auto it = EntityManager::components.find({ this->attachedScene, this, __typeid(T).qualifiedNameHash() });
        if (it != EntityManager::components.end())
        {
            delete it->second;
            EntityManager::components.erase(it);
        }
        else Debug::LogWarn("No identical component could be found on this Entity to be removed!");
    }
}