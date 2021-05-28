#include "Lock.h"

using namespace Marble;

void SpinLock::lock()
{
    while (locked.test_and_set(std::memory_order_acquire));
}
void SpinLock::unlock()
{
    locked.clear(std::memory_order_release);
}
SoyBoySpinLockGuard::SoyBoySpinLockGuard(SpinLock& lock)
{
    this->_lock = &lock;
    this->_lock->lock();
}
SoyBoySpinLockGuard::~SoyBoySpinLockGuard()
{
    this->_lock->unlock();
}

void YieldingLock::lock()
{
    while (locked.test_and_set(std::memory_order_acquire))
        std::this_thread::yield();
}
void YieldingLock::unlock()
{
    locked.clear(std::memory_order_release);
}
SoyBoyYieldingLockGuard::SoyBoyYieldingLockGuard(YieldingLock& lock)
{
    this->_lock = &lock;
    this->_lock->lock();
}
SoyBoyYieldingLockGuard::~SoyBoyYieldingLockGuard()
{
    this->_lock->unlock();
}