#pragma once

#include "threadcpp11.h"
#include <exception>
#include "../../_extrernal/sources.h"
#include "threadinter.h"

const class Export ThreadHandler{
public:
    ThreadHandler(CPPVersion version = CPP11);
     ~ThreadHandler();

     // create thread
     THandler createThread(threadcall callback, void* param);

     // start thread
     void start();

     // thread join
     void join();

     // thread sleep
     static bool sleep(uint32 milliseconds);

     // detach
     void detach();

private:
    static ThreadInterface *m_thread;
    CPPVersion m_version;
};

