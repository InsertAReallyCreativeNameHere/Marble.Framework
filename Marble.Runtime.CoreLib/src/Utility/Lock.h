#pragma once

#include <inc.h>

#include <atomic>

namespace Marble
{
    class coreapi SpinLock final
    {
        std::atomic_flag locked = ATOMIC_FLAG_INIT;
    public:
        void lock();
        void unlock();
    };
    class coreapi SoyBoySpinLockGuard final
    {
        SpinLock* _lock;
    public:
        SoyBoySpinLockGuard(SpinLock& lock);
        ~SoyBoySpinLockGuard();
    };
    
    class coreapi YieldingLock final
    {
        std::atomic_flag locked = ATOMIC_FLAG_INIT;
    public:
        void lock();
        void unlock();
    };
    class coreapi SoyBoyYieldingLockGuard final
    {
        YieldingLock* _lock;
    public:
        SoyBoyYieldingLockGuard(YieldingLock& lock);
        ~SoyBoyYieldingLockGuard();
    };
}
