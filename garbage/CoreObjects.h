#pragma once

#include <vector>
#include <Core/Components/RectTransform.h>

using namespace Core;
using namespace Core::UI;

namespace Core
{
    namespace Internal
    {
        namespace Objects
        {
            class coreapi Component : public Object
            {
                GameObject attachedGameObject;
                RectTransform attachedRectTransform;
            public:
                GameObject& gameObject();
                RectTransform& rectTransform();

                friend class GameObject;
            };
        }
    }

    using namespace Internal::Objects;

    class coreapi GameObject final
    {
        GameObject();
        ~GameObject();

        std::vector<Component*> components;
    public:
        RectTransform& rectTransform();
    };
};