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
            MemoryIdentifier* instanceID;
        protected:
            Object();
            virtual ~Object() = 0;
        public:
            inline InstanceID getInstanceID()
            {
                return (InstanceID)this->instanceID;
            }
        };
    }
}
