#pragma once

#ifndef __LOCKED_H__
#define __LOCKED_H__

#include <inc.h>

namespace Marble
{
    template<typename T>
    class coreapi Locked final
    {
        uint* currentAccesses;
        std::mutex* accMtx;

        T* value;
        std::mutex* mtx;

        bool operator==(const Locked<T>& rhs)
        {
            return this->value == rhs.value;
        }
        bool operator!=(const Locked<T>& rhs)
        {
            return this->value != rhs.value;
        }
    public:
        Locked()
        {
            this->value = new T();
            this->mtx = new std::mutex();
            this->currentAccesses = new uint(0);
            this->accMtx = new std::mutex();
        }
        Locked(const T& copy)
        {
            this->value = new T(copy);
            this->mtx = new std::mutex();
            this->currentAccesses = new uint(0);
            this->accMtx = new std::mutex();
        }
        ~Locked()
        {
            while (*this->currentAccesses != 0) { }
            delete this->currentAccesses;
            delete this->accMtx;
            delete this->value;
            delete this->mtx;
        }

        T& unsafeAccess()
        {
            return *this->value;
        }
    };

    template<typename T>
    struct coreapi LockWrite final
    {
        LockWrite(Locked<T>& lockedValue)
        {
            lockedValue.accMtx->lock();
            (*lockedValue.currentAccesses)++;
            lockedValue.accMtx->unlock();
            this->val = &lockedValue;
        }
        ~LockWrite()
        {
            this->val->accMtx->lock();
            (*this->val->currentAccesses)--;
            this->val->accMtx->unlock();
        }

        T* operator*()
        {
            std::lock_guard(this->val->mtx);
            return val->value;
        }
        T* operator->()
        {
            std::lock_guard(this->val->mtx);
            return val->value;
        }

        T& unsafeAccess()
        {
            return *this->val->value;
        }
    private:
        Locked<T>* val;
    };

    template<typename T>
    struct coreapi LockRead final
    {
        LockRead(Locked<T>& lockedValue)
        {
            lockedValue.accMtx->lock();
            (*lockedValue.currentAccesses)++;
            lockedValue.accMtx->unlock();
            this->val = &lockedValue;
        }
        ~LockRead()
        {
            this->val->accMtx->lock();
            (*this->val->currentAccesses)--;
            this->val->accMtx->unlock();
        }

        T operator()()
        {
            std::lock_guard(this->val->mtx);
            return *val->value;
        }

        T& unsafeAccess()
        {
            return *this->val->value;
        }
    private:
        Locked<T>* val;
    };
}

#endif