#pragma once

#include <mutex>
#include "../../_extrernal/sources.h"

class Export Condition
{
public:
    Condition();
    ~Condition();

    void wait(std::mutex &mutex);

    void signal();

    void broadcast();
private:
    std::condition_variable m_cond;
};
