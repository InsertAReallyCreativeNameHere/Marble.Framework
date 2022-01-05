#include "Component.h"

#include <Objects/Entity.h>

using namespace Marble;
using namespace Marble::Internal;

Component::~Component()
{
    if (this->eraseIteratorOnDestroy)
        this->attachedEntity->components.erase(this->it);
}

void Component::setIndex(size_t value)
{
    if (value < this->attachedEntity->components.size())
    {
        this->attachedEntity->components.splice
        (
            std::next(this->attachedEntity->components.begin(), value),
            this->attachedEntity->components, this->it
        );
        this->_index = value;
        auto it = std::next(this->it);
        while (it != this->attachedEntity->components.end())
        {
            ++(*it)->_index;
            ++it;
        }
    }
}
