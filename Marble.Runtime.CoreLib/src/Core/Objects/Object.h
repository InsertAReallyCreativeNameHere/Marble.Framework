#pragma once

#include "inc.h"

#include <iostream>

namespace Marble
{
    namespace Internal
    {
        struct MemoryIdentifier
        {
        };

        typedef uintptr_t InstanceID;
        
        class coreapi Object
        {
            MemoryIdentifier* instanceID = new MemoryIdentifier();
        protected:
            inline virtual ~Object()
            {
                delete this->instanceID;
            }
        public:
            inline InstanceID getInstanceID()
            {
                return (InstanceID)this->instanceID;
            }
        };
    }
}
