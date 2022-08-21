#pragma once

#include <thread>
#include "threadinter.h"

using namespace std;

class ThreadCpp11 : public ThreadInterface
{
public:
    ThreadCpp11();
    ~ThreadCpp11();
    // create thread
    virtual THandler createThread(threadcall, void*);

    // start thread
    virtual void start();

    // thread join
    virtual void join();

    // thread sleep
    virtual bool sleep(uint32 milliseconds);

    // detach
    virtual void detach();
private:
    thread m_thread;
};