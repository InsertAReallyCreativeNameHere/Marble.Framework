#pragma once

#include <array>
#include <cstdint>
#include <utility>
#include <Utility/MemoryChunk.h>

namespace Marble
{
    template <typename T>
    class MinAllocList
    {
        struct Node
        {
            std::array<MemoryChunk<T>, 4096> arr;
            size_t arrNext = 1;
            std::array<size_t, 4096> indexStack;
            size_t stackNext = 0;
        };

        std::list<Node> nodes;
    public:
        struct Element
        {
            inline Element() = default;
            inline Element(std::list<Node>::iterator it, size_t index) : it(it), index(index)
            {
            }

            inline T& operator*()
            {
                return *it->arr[this->index].ptr();
            }
            inline T* operator->()
            {
                return it->arr[this->index].ptr();
            }
        private:
            std::list<Node>::iterator it;
            size_t index;
        };

        inline MinAllocList() = default;

        template <typename... Args>
        inline Element emplace(Args&&... args)
        {
            for (auto it = this->nodes.begin(); it != this->nodes.end(); ++it)
            {
                if (it->stackNext != 4096 && it->stackNext != 0)
                {
                    new(it->arr[it->indexStack[it->stackNext]].ptr()) T(std::forward<Args>(args)...);
                    return { it, it->indexStack[it->stackNext--] };
                }
                if (it->arrNext != 4096)
                {
                    new(it->arr[it->arrNext].ptr()) T(std::forward<Args>(args)...);
                    return { it, it->arrNext++ };
                }
            }

            this->nodes.emplace_back();
            auto it = --this->nodes.end();
            new (it->arr[0].ptr()) T(std::forward<Args>(args)...);
            return { it, 0 };
        }
        inline void erase(Element it)
        {
            it->it->arr[it->index].ptr()->~T();
            it->it->indexStack[it->it->stackNext++] = it->index;
            if (it->it->arrNext == it->it->stackNext)
                this->nodes.erase(it->it);
        }

        inline bool empty()
        {
            return this->nodes.empty();
        }
    };
}