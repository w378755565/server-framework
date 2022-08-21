#include "threadhandler.h"

ThreadInterface* ThreadHandler::m_thread = nullptr;

ThreadHandler::ThreadHandler(CPPVersion version)
{
    m_version = version;
    switch (m_version)
    {
    case CPP11:
        m_thread = new ThreadCpp11();
    default:
        throw new std::exception("creating the thread handler is error, the verson is not support.");
    }
}

ThreadHandler::~ThreadHandler()
{
    delete m_thread;
}

// create thread
THandler ThreadHandler::createThread(threadcall callback, void* param)
{
    return m_thread->createThread(callback, param);
}

// start thread
void ThreadHandler::start()
{
    m_thread->start();
}

// thread join
void ThreadHandler::join()
{
    m_thread->join();
}

// thread sleep
bool ThreadHandler::sleep(uint32 milliseconds)
{
    if (m_thread == nullptr) {
        return false;
    }
    return m_thread->sleep(milliseconds);
}

// detach
void ThreadHandler::detach()
{
    m_thread->detach();
}
