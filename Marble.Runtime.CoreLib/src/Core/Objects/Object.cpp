#include "Object.h"

using namespace Marble::Internal;

Object::Object()
{
    this->instanceID = new MemoryIdentifier();
}
Object::~Object()
{
    delete this->instanceID;
}

uintptr_t Object::getInstanceID()
{
    return (uintptr_t)this->instanceID;
}
