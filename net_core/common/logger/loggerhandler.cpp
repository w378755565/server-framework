#include "loggerhandler.h"

bool LoggerHandler::init(LoggerOut outType, bool sync)
{
    m_outType = outType;
    m_sync = sync;
    if (m_sync) {
        m_thread = new ThreadHandler();
    }
    return true;
}

LoggerHandler::~LoggerHandler()
{
    delete m_log_buf;
    delete m_thread;
}
void LoggerHandler::debug(const char* format, ...)
{

}

void LoggerHandler::info(const char* format, ...)
{

}

void LoggerHandler::error(const char* format, ...)
{

}

void LoggerHandler::warning(const char* format, ...)
{

}