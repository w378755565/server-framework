#pragma once

#include <mutex>
#include "../../_extrernal/sources.h"

const class Export Locker {
public:
    Locker(CPPVersion version = CPP11);
    ~Locker();

    // lock
    bool lockGuard();

    // unlock
    bool unlockGuard();

    // lock time out
    bool tryLock();

    bool lock();

    bool unlock();

    std::mutex& getGuardMutex();

    std::mutex& getMutex();

    std::timed_mutex& getTimedMutex();

    // 
private:
    CPPVersion m_version;

    // graud lock
    std::lock_guard<std::mutex>* m_guard_lock;

    // mutex
    std::mutex m_guard_mutex;
    std::mutex m_mutex;
    std::timed_mutex m_timed_mutex;
};