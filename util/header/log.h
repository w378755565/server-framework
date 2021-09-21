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

/* ��־�࣬���� */
class Log
{
public:
    // ��ȡ����
    static Log& get_instance()
    {
        static Log log;
        return log;
    }
    // ˢ����־�̵߳Ĵ��������������������е������Ƿ����
    static void* flush_log_thread(void* arg)
    {
        Log::get_instance().async_write_log();
        return NULL;
    }
    // ��ʼ������
    bool init(string& file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);
    // д��־��Ϣ
    void write_log(int level, const char* format, ...);
    // ˢ�»�����
    void flush();
    // ���������ر���Դ
    ~Log();
private:
    Log();
    // �첽д������е���־��Ϣ���ļ�
    void* async_write_log()
    {
        string single_log;
        // �������ȴ�
        while (m_log_queue->pop(single_log))
        {
            // һ��������
            m_mutex.lock();
            // д��
            fputs(single_log.c_str(), m_file);
            m_mutex.unlock();
        }
        return NULL;
    }
    // ��־�ļ�·��
    string dir_name;
    // ��־�ļ�����
    string log_name;
    //��־�������
    int m_split_lines;
    //��־��������С
    int m_log_buf_size;
    // ������
    char* m_buf;
    //��־������¼
    long long m_count;
    //��Ϊ�������,��¼��ǰʱ������һ��
    int m_today;
    // �ļ�ָ��
    FILE* m_file;
    // ����
    Lock m_mutex;
    // �ļ�д���Ƿ����첽
    bool is_async;
    //��������
    block_queue<string>* m_log_queue;
    // �Ƿ�ر���־
    bool close_log;
};

#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::get_instance().write_log(0, format, ##__VA_ARGS__); Log::get_instance().flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::get_instance().write_log(1, format, ##__VA_ARGS__); Log::get_instance().flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::get_instance().write_log(2, format, ##__VA_ARGS__); Log::get_instance().flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::get_instance().write_log(3, format, ##__VA_ARGS__); Log::get_instance().flush();}

#endif
