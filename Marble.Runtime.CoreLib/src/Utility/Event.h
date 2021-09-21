#pragma once

#include "inc.h"

#include <vector>
#include <skarupke/function.h>

namespace Marble
{
    template <typename... Args>
    class Event
    {
        std::vector<skarupke::function<void(Args...)>> children;
    public:
        inline Event()
        {
        }
        Event(const Event<Args...>&) = delete;
        Event(Event<Args...>&&) = delete;

        inline void operator+=(const skarupke::function<void(Args...)>& func)
        {
            this->children.push_back(func);
        }
        inline void operator+=(skarupke::function<void(Args...)>&& func)
        {
            this->children.push_back(func);
        }
        inline void operator-=(const skarupke::function<void(Args...)>& func)
        {
            for (auto it = this->children.begin(); it != this->children.end(); ++it)
            {
                if (*it == func)
                {
                    this->children.erase(it);
                    break;
                }
            }
        }
        inline void operator()(Args... args)
        {
            for (auto it = this->children.begin(); it != this->children.end(); ++it)
                (*it)(args...);
        }

        inline bool unhandled()
        {
            return this->children.empty();
        }
        inline void clear()
        {
            this->children.clear();
        }
    };
    
    template <typename... Args>
    class FuncPtrEvent
    {
        std::vector<void (*)(Args...)> children;
    public:
        inline FuncPtrEvent()
        {
        }
        FuncPtrEvent(const FuncPtrEvent<Args...>&) = delete;
        FuncPtrEvent(FuncPtrEvent<Args...>&&) = delete;

        inline void operator+=(void (*func)(Args...))
        {
            this->children.push_back(func);
        }
        inline void operator-=(void (*func)(Args...))
        {
            for (auto it = this->children.begin(); it != this->children.end(); ++it)
            {
                if (*it == func)
                {
                    this->children.erase(it);
                    break;
                }
            }
        }
        inline void operator()(Args... args)
        {
            for (auto it = this->children.begin(); it != this->children.end(); ++it)
                (*it)(args...);
        }

        inline bool unhandled()
        {
            return this->children.empty();
        }
        inline void clear()
        {
            this->children.clear();
        }
    };
}