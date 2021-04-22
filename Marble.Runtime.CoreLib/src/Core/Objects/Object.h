#pragma once

#include <inc.h>

#include <iostream>

namespace Marble
{
    namespace Internal
    {
        struct coreapi MemoryIdentifier final
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
            InstanceID getInstanceID();
        };
    }
}
