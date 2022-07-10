#pragma once

namespace Marble
{
    template <typename T>
    struct MemoryChunk
    {
        alignas(T) unsigned char mem[sizeof(T)];

        inline T* ptr()
        {
            return reinterpret_cast<T*>(&this->mem);
        }

        inline operator T& ()
        {
            return *reinterpret_cast<T*>(&this->mem);
        }
    };
}