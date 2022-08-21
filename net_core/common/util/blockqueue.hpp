#pragma once

#include "../lock/condition.h"
#include "../lock/locker.h"
#include "../../_extrernal/sources.h"

template<class T>
class Export BlockQueue
{
public:
    BlockQueue(int32 maxSize = 1000)
    {
        if (maxSize <= 0) 
        {
            exit(-1);
        }
        m_maxSize = maxSize;
        m_queue = new T[m_maxSize];
        m_size = 0;
        m_front = m_back = -1;
    }
    ~BlockQueue()
    {
        m_lock.lock();
        if (m_queue != nullptr) {
            delete[]m_queue;
        }
        m_lock.unlock();
    }

    void clear()
    {
        m_lock.lock();
        m_size = 0;
        m_front = -1;
        m_back = -1;
        m_lock.unlock();
    }
    bool pop(T &val)
    {
        m_lock.lock();
        while (m_size <= 0) {

        }
        m_lock.unlock();
    }
    bool push()
    {

    }
    bool full()
    {

    }
    bool empty()
    {

    }
    bool front()
    {

    }
    bool back()
    {

    }
    int32 size()
    {

    }
    int32 maxSize()
    {

    }

private:
    T *m_queue;
    int32 m_maxSize;

    int32 m_size;
    int32 m_front;
    int32 m_back;

    Locker m_lock;
    Condition m_condition;
};

