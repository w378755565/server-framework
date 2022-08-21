#pragma once

// thread interface
#include "../../_extrernal/sources.h"

class Export ThreadInterface {
public:
    virtual ~ThreadInterface() {}

    // create thread
    virtual THandler createThread(threadcall, void*) = 0;

    // start thread
    virtual void start() = 0;

    // thread join
    virtual void join() = 0;

    // thread sleep
    virtual bool sleep(uint32) = 0;

    // detach
    virtual void detach() = 0;

protected:

    threadcall m_callback;
    void* m_param;

    // thread ctrl
    bool m_control;
private:
    CPPVersion m_version;
};

