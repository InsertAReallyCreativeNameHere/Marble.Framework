#include "Object.h"

using namespace Marble::Internal;

Object::Object() : instanceID(new MemoryIdentifier())
{
}
Object::~Object()
{
    delete this->instanceID;
}
