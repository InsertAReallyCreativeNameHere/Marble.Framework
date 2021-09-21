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
        Event()
        {
        }
        Event(const Event<Args...>&) = delete;
        Event(Event<Args...>&&) = delete;

        void operator+=(const skarupke::function<void(Args...)>& func)
        {
            this->children.push_back(func);
        }
        void operator+=(skarupke::function<void(Args...)>&& func)
        {
            this->children.push_back(func);
        }
        void operator-=(const skarupke::function<void(Args...)>& func)
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
        void operator()(Args... args)
        {
            for (auto it = this->children.begin(); it != this->children.end(); ++it)
                (*it)(args...);
        }

        bool unhandled()
        {
            return this->children.empty();
        }
        void clear()
        {
            this->children.clear();
        }
    };
    
    template <typename... Args>
    class FuncPtrEvent
    {
        std::vector<void (*)(Args...)> children;
    public:
        FuncPtrEvent()
        {
        }
        FuncPtrEvent(const FuncPtrEvent<Args...>&) = delete;
        FuncPtrEvent(FuncPtrEvent<Args...>&&) = delete;

        void operator+=(void (*func)(Args...))
        {
            this->children.push_back(func);
        }
        void operator-=(void (*func)(Args...))
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
        void operator()(Args... args)
        {
            for (auto it = this->children.begin(); it != this->children.end(); ++it)
                (*it)(args...);
        }

        bool unhandled()
        {
            return this->children.empty();
        }
        void clear()
        {
            this->children.clear();
        }
    };
}