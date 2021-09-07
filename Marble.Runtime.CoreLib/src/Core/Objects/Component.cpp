#include "Component.h"

#include <Core/Objects/Entity.h>

using namespace Marble;
using namespace Marble::Internal;

Component::Component()
{
}
Component::~Component()
{
    if (this->eraseIteratorOnDestroy)
        this->attachedEntity->components.erase(this->it);
}
