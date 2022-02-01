#pragma once

#include <type_traits>

namespace Marble
{
    template <typename T, size_t MaxSize>
    class StaticQueue
    {
        struct ObjectMemory
        {
            alignas(T) char mem[sizeof(T)];

            inline T* object()
            {
                return reinterpret_cast<T*>(&this->mem);
            }
        };

        ObjectMemory data[MaxSize];
        size_t len = 0;
    public:
        template <typename... Args>
        void emplaceBack(Args&&... args)
        { new(&this->data[this->len++].mem) T(std::forward<Args>(args)...); }
        T&& popBack()
        {
            if constexpr (std::is_trivially_destructible<T>::value)
                return std::move(*this->data[--len].object());
            else
            {
                T&& ret = std::move(this->data[--len]);
                this->data[len].object()->~T();
                return ret;
            }
        }

        size_t length()
        {
            return this->len;
        }
        T& operator[](size_t index)
        {
            return *this->data[index].object();
        }
    };
}