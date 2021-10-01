#include "Component.h"

#include <Objects/Entity.h>

using namespace Marble;
using namespace Marble::Internal;

Component::~Component()
{
    if (this->eraseIteratorOnDestroy)
        this->attachedEntity->components.erase(this->it);
}
