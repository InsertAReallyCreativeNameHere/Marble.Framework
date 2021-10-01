#pragma once

#include "inc.h"
#include "Marble.Runtime.CoreLib.Exports.h"

#include <iostream>

namespace Marble
{
    namespace Internal
    {
        struct MemoryIdentifier
        {
        };

        typedef uintptr_t InstanceID;
        
        class __marble_corelib_api Object
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
