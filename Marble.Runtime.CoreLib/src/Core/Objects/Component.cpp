#include "Component.h"

using namespace Marble;
using namespace Marble::Internal;

Component::Component() :
onDestroy
(
    [](Component* _this)
    {
        for (auto it = _this->attachedEntity->components.begin(); it != _this->attachedEntity->components.end(); ++it)
        {
            _this->attachedEntity->components.erase(it);
            break;
        }
    }
)
{
}
Component::~Component()
{
    this->onDestroy(this);
}

Entity* Component::entity()
{
    return this->attachedEntity;
}
RectTransform* Component::rectTransform()
{
    return this->attachedEntity->attachedRectTransform;
}
