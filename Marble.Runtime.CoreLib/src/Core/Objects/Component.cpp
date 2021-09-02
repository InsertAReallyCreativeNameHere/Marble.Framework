#include "Component.h"

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
