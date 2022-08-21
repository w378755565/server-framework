#include "condition.h"

Condition::Condition()
{

}

Condition::~Condition()
{

}

void Condition::wait(std::mutex& mutex)
{
    std::unique_lock<std::mutex> lk(mutex);
    m_cond.wait(lk);
}

void Condition::signal()
{
    m_cond.notify_one();
}

void Condition::broadcast()
{
    m_cond.notify_all();
}
