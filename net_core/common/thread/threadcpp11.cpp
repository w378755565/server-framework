#include "threadcpp11.h"

ThreadCpp11::ThreadCpp11()
{
    m_control = false;
}

ThreadCpp11::~ThreadCpp11()
{
    m_control = false;
}

// create thread
THandler ThreadCpp11::createThread(threadcall callback, void* param)
{
    m_callback = callback;
    m_param = param;
    return nullptr;
}

// start thread
void ThreadCpp11::start()
{
    m_control = true;
    m_thread = thread(m_callback, m_param);
}

// thread join
void ThreadCpp11::join()
{
    if (m_control) {
        m_thread.join();
    }
}

// thread sleep
bool ThreadCpp11::sleep(uint32 milliseconds)
{
    if (m_control) {
        this_thread::sleep_for(chrono::milliseconds(milliseconds));
        return true;
    }
    return false;
}

// detach
void ThreadCpp11::detach()
{
    if (m_control) {
        m_thread.detach();
    }
}

