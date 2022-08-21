#pragma once

#include <fstream>
#include "../../_extrernal/sources.h"
#include "../thread/threadhandler.h"
#include "../lock/locker.h"
#include "../lock/condition.h"

// the class for logger
// LoggerHandler logger = LoggerHandler.getLogger();
// logger.init();
// logger.debug("%s", params...);
class LoggerHandler 
{
public:
    // singleton
    static LoggerHandler& getLogger()
    {
        static LoggerHandler logger;
        return logger;
    }
    bool init(LoggerOut outType, bool sync);
    ~LoggerHandler();
    void debug(const char* format, ...);
    void info(const char* format, ...);
    void error(const char* format, ...);
    void warning(const char* format, ...);
private:
    LoggerHandler();
    // sync to write logger.
    static void* m_sync_write_logger(void* args)
    {

    }
    enum LoggerLevel {
        Info,
        Debug,
        Warn,
        Error
    };
    // write logger
    void write_log(LoggerLevel level, const char* format, ...);
private:
    // write log thread.
    ThreadHandler* m_thread;
    // if use multi thread
    bool m_sync;
    // log buffer
    char* m_log_buf;
    // buf max size
    uint32 m_bufMaxSize;
    // file max line
    uint32 m_file_max_line;
    // log control
    bool m_control;
    // ofstream
    std::ofstream* m_file;
    std::ostream* m_console;
    Locker m_lock;
    Condition m_condition;

    // the out type, console or file
    LoggerOut m_outType;
};