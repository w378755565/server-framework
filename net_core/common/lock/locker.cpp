#include "locker.h"

Locker::Locker(CPPVersion version)
{
    m_version = version;
}


Locker::~Locker()
{
    if (m_guard_lock != nullptr) {
        delete m_guard_lock;
    }
}

// lock
bool Locker::lockGuard()
{
    m_guard_lock = new std::lock_guard<std::mutex>(m_guard_mutex);
    return true;
}

// unlock
bool Locker::unlockGuard()
{
    delete m_guard_lock;
    m_guard_lock = nullptr;
    return true;
}

bool Locker::lock()
{
    m_mutex.lock();
    return true;
}

bool Locker::unlock()
{
    m_mutex.unlock();
    return true;
}

bool Locker::tryLock()
{
    m_mutex.try_lock();
    return true;
}

std::mutex& Locker::getGuardMutex()
{
    return m_guard_mutex;
}

std::mutex& Locker::getMutex()
{
    return m_mutex;
}

std::timed_mutex& Locker::getTimedMutex()
{
    return m_timed_mutex;
}