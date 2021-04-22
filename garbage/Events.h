#pragma once

#include "inc.h"

#include <list>

namespace Marble
{
    // Thanks to this wonderful page https://schneegans.github.io/tutorials/2015/09/20/signal-slots.
    // This is modified of course.
    // It explains the signal-slots system very well and provides this excellent example.

    // A signal object may call multiple slots with the
    // same signature. You can connect functions to the signal
    // which will be called when the emit() method on the
    // signal object is invoked. Any argument passed to emit()
    // will be passed to the given functions.

    template <typename... Args>
    class coreapi Signal final
    {
    public:
        Signal() { }

        // Copy creates new signal.
        Signal(Signal const& other)
        {
            disconnectAll();
            *this = other;
        }

        // Connects a std::function pointer to the signal.
        void connect(const std::function<void(Args...)>& func) const
        {
            slots.push_back(&func);
        }
        // Same as connect.
        void operator+=(const std::function<void(Args...)>& rhs) const
        {
            slots.push_back(&rhs);
        }

        // Disconnects a previously connected function
        void disconnect(const std::function<void(Args...)>& func) const
        {
            slots.remove(&func);
        }
        // Same as disconnect.
        void operator-=(const std::function<void(Args...)>& rhs) const
        {
            slots.remove(&rhs);
        }

        // Disconnects all previously connected functions
        void disconnectAll() const {
            slots.clear();
        }

        // Calls all connected functions
        void emit(Args... p)
        {
            for (int i = 0; i < slots.size(); i++)
            {
                (**std::next(slots.begin(), i))(std::forward<Args>(p)...);
            }
        }
        // Same as emit();
        void operator()(Args... p)
        {
            for (int i = 0; i < slots.size(); i++)
            {
                (**std::next(slots.begin(), i))(std::forward<Args>(p)...);
            }
        }

        // assignment creates new Signal
        Signal& operator=(Signal const& other)
        {
            disconnectAll();
            this->slots = other.slots;
            return *this;
        }

        uint slotsCount()
        {
            return this->slots.size();
        }
    private:
        mutable std::list<const std::function<void(Args...)>*> slots;
    };

    /*class EventPublisher
    {
        std::vector<EventSubscriber> subscribers = std::vector<EventSubscriber>();
    protected:
        template<typename... Args>
        void operator() (Args... args)
        {

        }
    };

    struct EventSubscriber
    {
    protected:
        EventSubscriber()
        {

        }

        template<typename... Args>
        void operator() (Args... args)
        {

        }
    };

    class TestSub : public EventSubscriber
    {

    };
    */
    /*template <typename TFunc>
    class Event;

    template <class RetType, class... Args>
    class Event<RetType(Args ...)> final
    {
    private:
        typedef typename std::function<RetType(Args ...)> Closure;

        struct ComparableClosure
        {
            Closure Callable;
            void* Object;
            uint8_t* Functor;
            int FunctorSize;

            ComparableClosure(const ComparableClosure&) = delete;

            ComparableClosure() : Object(nullptr), Functor(nullptr), FunctorSize(0) { }

            ComparableClosure(Closure&& closure) : Callable(std::move(closure)), Object(nullptr), Functor(nullptr), FunctorSize(0) { }

            ~ComparableClosure()
            {
                if (Functor != nullptr)
                    delete[] Functor;
            }

            ComparableClosure& operator=(const ComparableClosure& closure)
            {
                Callable = closure.Callable;
                Object = closure.Object;
                FunctorSize = closure.FunctorSize;
                if (closure.FunctorSize == 0)
                {
                    Functor = nullptr;
                }
                else
                {
                    Functor = new uint8_t[closure.FunctorSize];
                    std::memcpy(Functor, closure.Functor, closure.FunctorSize);
                }

                return *this;
            }

            bool operator==(const ComparableClosure& closure)
            {
                if (Object == nullptr && closure.Object == nullptr)
                {
                    return Callable.target_type() == closure.Callable.target_type();
                }
                else
                {
                    return Object == closure.Object && FunctorSize == closure.FunctorSize
                        && std::memcmp(Functor, closure.Functor, FunctorSize) == 0;
                }
            }
        };

        struct ClosureList
        {
            ComparableClosure* Closures;
            int Count;

            ClosureList(ComparableClosure* closures, int count)
            {
                Closures = closures;
                Count = count;
            }

            ~ClosureList()
            {
                delete[] Closures;
            }
        };

        typedef std::shared_ptr<ClosureList> ClosureListPtr;

    private:
        ClosureListPtr m_events;

    private:
        bool addClosure(const ComparableClosure& closure)
        {
            auto events = std::atomic_load(&m_events);
            int count;
            ComparableClosure* closures;
            if (events == nullptr)
            {
                count = 0;
                closures = nullptr;
            }
            else
            {
                count = events->Count;
                closures = events->Closures;
            }

            auto newCount = count + 1;
            auto newClosures = new ComparableClosure[newCount];
            if (count != 0)
            {
                for (int i = 0; i < count; i++)
                    newClosures[i] = closures[i];
            }

            newClosures[count] = closure;
            auto newEvents = ClosureListPtr(new ClosureList(newClosures, newCount));
            if (std::atomic_compare_exchange_weak(&m_events, &events, newEvents))
                return true;

            return false;
        }

        bool removeClosure(const ComparableClosure& closure)
        {
            auto events = std::atomic_load(&m_events);
            if (events == nullptr)
                return true;

            int index = -1;
            auto count = events->Count;
            auto closures = events->Closures;
            for (int i = 0; i < count; i++)
            {
                if (closures[i] == closure)
                {
                    index = i;
                    break;
                }
            }

            if (index == -1)
                return true;

            auto newCount = count - 1;
            ClosureListPtr newEvents;
            if (newCount == 0)
            {
                newEvents = nullptr;
            }
            else
            {
                auto newClosures = new ComparableClosure[newCount];
                for (int i = 0; i < index; i++)
                    newClosures[i] = closures[i];

                for (int i = index + 1; i < count; i++)
                    newClosures[i - 1] = closures[i];

                newEvents = ClosureListPtr(new ClosureList(newClosures, newCount));
            }

            if (std::atomic_compare_exchange_weak(&m_events, &events, newEvents))
                return true;

            return false;
        }

    public:
        Event()
        {
            std::atomic_store(&m_events, ClosureListPtr());
        }

        Event(const Event& event)
        {
            std::atomic_store(&m_events, std::atomic_load(&event.m_events));
        }

        ~Event()
        {
            (*this) = nullptr;
        }

        void operator =(const Event& event)
        {
            std::atomic_store(&m_events, std::atomic_load(&event.m_events));
        }

        void operator=(nullptr_t nullpointer)
        {
            while (true)
            {
                auto events = std::atomic_load(&m_events);
                if (!std::atomic_compare_exchange_weak(&m_events, &events, ClosureListPtr()))
                    continue;

                break;
            }
        }

        bool operator==(nullptr_t nullpointer)
        {
            auto events = std::atomic_load(&m_events);
            return events == nullptr;
        }

        bool operator!=(nullptr_t nullpointer)
        {
            auto events = std::atomic_load(&m_events);
            return events != nullptr;
        }

        void operator +=(Closure f)
        {
            ComparableClosure closure(std::move(f));
            while (true)
            {
                if (addClosure(closure))
                    break;
            }
        }

        void operator -=(Closure f)
        {
            ComparableClosure closure(std::move(f));
            while (true)
            {
                if (removeClosure(closure))
                    break;
            }
        }

        template <typename TObject>
        void Bind(RetType(TObject::* function)(Args...), TObject* object)
        {
            ComparableClosure closure;
            closure.Callable = [object, function](Args&&...args)
            {
                return (object->*function)(std::forward<Args>(args)...);
            };
            closure.FunctorSize = sizeof(function);
            closure.Functor = new uint8_t[closure.FunctorSize];
            std::memcpy(closure.Functor, (void*)&function, sizeof(function));
            closure.Object = object;

            while (true)
            {
                if (addClosure(closure))
                    break;
            }
        }

        template <typename TObject>
        void Unbind(RetType(TObject::* function)(Args...), TObject* object)
        {
            ComparableClosure closure;
            closure.FunctorSize = sizeof(function);
            closure.Functor = new uint8_t[closure.FunctorSize];
            std::memcpy(closure.Functor, (void*)&function, sizeof(function));
            closure.Object = object;

            while (true)
            {
                if (removeClosure(closure))
                    break;
            }
        }

        void operator()()
        {
            auto events = std::atomic_load(&m_events);
            if (events == nullptr)
                return;

            auto count = events->Count;
            auto closures = events->Closures;
            for (int i = 0; i < count; i++)
                closures[i].Callable();
        }

        template <typename TArg0, typename ...Args2>
        void operator()(TArg0 a1, Args2... tail)
        {
            auto events = std::atomic_load(&m_events);
            if (events == nullptr)
                return;

            auto count = events->Count;
            auto closures = events->Closures;
            for (int i = 0; i < count; i++)
                closures[i].Callable(a1, tail...);
        }
    };*/
    /*template<typename... T>
    class EventPublisher
    {
        std::vector<std::function<void(T...)>*> subscribers = std::vector<std::function<void(T...)>*>();
    public:
        void addSubscriber(const std::function<void(T...)> func)
        {
            std::function<void(T...)> f = func;
            subscribers.push_back(&f);
        }
        void removeSubscriber(const std::function<void(T...)> func)
        {
            for (int i = 0; i < subscribers.size(); i++)
            {
                if (subscribers[i] == &func)
                {
                    subscribers.erase(subscribers.begin() + i);
                }
            }
        }
        void clearSubscribers()
        {
            subscribers.clear();
        }

        void operator() ()
        {
            std::for_each
            (
                std::execution::par_unseq,
                subscribers.begin(),
                subscribers.end(),
                [](std::function<void(T...)>* item)
                {
                    (*item)();
                }
            );
        }
    };*/
    /*
    template<class... T>
    [event_source(native)]
    class EventSource
    {
    public:
        __event void evt(T... args);
    };

    template<class... U>
    [event_receiver(native)]
    class EventSubscriber
    {
    public:
        std::function<void(U...)> function;
        EventSubscriber(const std::function<void(U...)>& func) : function(func)
        {
        }

        void subscribeTo(EventSource* source)
        {
            __hook(&EventSource::)
        }
    };*/
}
