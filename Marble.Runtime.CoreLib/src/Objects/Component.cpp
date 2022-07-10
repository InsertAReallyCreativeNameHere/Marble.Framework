#include "Component.h"

#include <Objects/Entity.h>

using namespace Marble;
using namespace Marble::Internal;

Component::~Component()
{
    EntityManager::components.erase({ this->attachedEntity->attachedScene, this->attachedEntity, this->reflection.typeID });
}
