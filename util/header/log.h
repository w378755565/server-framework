#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <stdarg.h>
#include <pthread.h>
#include <string.h>
#include <cstdarg>
#include "block_queue.h"
#include "locker.h"

using namespace std;

/* 日志类，单例 */
class Log
{
public:
    // 获取单例
    static Log& get_instance()
    {
        static Log log;
        return log;
    }
    // 刷新日志线程的处理函数，监听阻塞队列中的数据是否存在
    static void* flush_log_thread(void* arg)
    {
        Log::get_instance().async_write_log();
        return NULL;
    }
    // 初始化函数
    bool init(string& file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);
    // 写日志信息
    void write_log(int level, const char* format, ...);
    // 刷新缓冲区
    void flush();
    // 析构函数关闭资源
    ~Log();
private:
    Log();
    // 异步写入队列中的日志信息到文件
    void* async_write_log()
    {
        string single_log;
        // 会锁定等待
        while (m_log_queue->pop(single_log))
        {
            // 一旦有数据
            m_mutex.lock();
            // 写入
            fputs(single_log.c_str(), m_file);
            m_mutex.unlock();
        }
        return NULL;
    }
    // 日志文件路径
    string dir_name;
    // 日志文件名称
    string log_name;
    //日志最大行数
    int m_split_lines;
    //日志缓冲区大小
    int m_log_buf_size;
    // 缓冲区
    char* m_buf;
    //日志行数记录
    long long m_count;
    //因为按天分类,记录当前时间是那一天
    int m_today;
    // 文件指针
    FILE* m_file;
    // 加锁
    Lock m_mutex;
    // 文件写入是否是异步
    bool is_async;
    //阻塞队列
    block_queue<string>* m_log_queue;
    // 是否关闭日志
    bool close_log;
};

#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::get_instance().write_log(0, format, ##__VA_ARGS__); Log::get_instance().flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::get_instance().write_log(1, format, ##__VA_ARGS__); Log::get_instance().flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::get_instance().write_log(2, format, ##__VA_ARGS__); Log::get_instance().flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::get_instance().write_log(3, format, ##__VA_ARGS__); Log::get_instance().flush();}

#endif
