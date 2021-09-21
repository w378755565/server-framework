#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include "log.h"
#include <pthread.h>
#include <iostream>

Log::Log()
{
	m_count = 0;
	is_async = false;
}

bool Log::init(string& file_name, int close_log, int log_buf_size, int split_lines, int max_queue_size)
{
	cout << file_name << endl;
	// 如果最大的队列大小存在，则表示为异步记录日志
	if (max_queue_size > 0)
	{
		cout << "start thread" << endl;
		this->m_log_queue = new block_queue<string>(max_queue_size);
		this->is_async = true;
		// 开启线程，回调函数为flush_log_thread，对队列进行数据写入
		pthread_t pid;
		pthread_create(&pid, NULL, flush_log_thread, NULL);
	}
	// 设置关闭标志
	this->close_log = close_log;
	// 设置日志行数
	this->m_split_lines = split_lines;
	// 设置缓冲区大小用来写入文件
	this->m_log_buf_size = log_buf_size;
	cout << "new memory" << endl;
	// 设置缓冲区
	m_buf = new char[m_log_buf_size];
	memset(m_buf, '\0', m_log_buf_size);
	cout << "find" << endl;
	// 将文件名分割为路径和名称
	int last_index = file_name.find_last_of("/");

	cout << last_index << endl;

	time_t t = time(NULL);
	struct tm* sys_tm = localtime(&t);
	struct tm my_tm = *sys_tm;

	// 文件全路径
	char log_full_name[256] = { 0 };
	// 没有斜杠，表示失败
	if (last_index == -1)
	{
		// 设置目录
		snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name.c_str());
	}
	else
	{
		// 进行划分得到目录
		this->dir_name = file_name.substr(0, last_index);
		// 得到文件名
		if (last_index + 1 < file_name.size())
			this->log_name = file_name.substr(last_index + 1);
		snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", dir_name.c_str(), my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name.c_str());
	}
	m_today = my_tm.tm_mday;

	cout << "log_full_name:" << log_full_name << endl;

	cout << "open file" << endl;

	// 打开文件，没有文件则创建
	m_file = fopen(log_full_name, "a");
	if (m_file == NULL)
	{
		return false;
	}

	return true;
}

void Log::write_log(int level, const char* format, ...)
{
	struct timeval now = { 0, 0 };
	gettimeofday(&now, NULL);
	time_t t = now.tv_sec;
	struct tm* sys_tm = localtime(&t);
	struct tm my_tm = *sys_tm;
	char s[16] = { 0 };
	// 根据类型写日志
	switch (level)
	{
	case 0:
		strcpy(s, "[debug]:");
		break;
	case 1:
		strcpy(s, "[info]:");
		break;
	case 2:
		strcpy(s, "[warn]:");
		break;
	case 3:
		strcpy(s, "[erro]:");
		break;
	default:
		strcpy(s, "[info]:");
		break;
	}
	//写入一个log，加锁
	m_mutex.lock();
	// 写入行数加1
	m_count++;
	// 如果是第二天，则重新刷新一行，并且需要重新设置文件
	if (m_today != my_tm.tm_mday || m_count % m_split_lines == 0)
	{
		// 写入
		char new_log[256] = { 0 };
		fflush(m_file);
		fclose(m_file);
		char tail[16] = { 0 };

		snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);

		if (m_today != my_tm.tm_mday)
		{
			snprintf(new_log, 255, "%s%s%s", dir_name.c_str(), tail, log_name.c_str());
			m_today = my_tm.tm_mday;
			m_count = 0;
		}
		else
		{
			snprintf(new_log, 255, "%s%s%s.%lld", dir_name.c_str(), tail, log_name.c_str(), m_count / m_split_lines);
		}
		m_file = fopen(new_log, "a");
	}
	// 解锁
	m_mutex.unlock();
	// 对所有的参数进行循环
	va_list valst;
	va_start(valst, format);

	string log_str;
	m_mutex.lock();
	//写入的具体时间内容格式
	int n = snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
		my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
		my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);

	int m = vsnprintf(m_buf + n, m_log_buf_size - 1, format, valst);
	m_buf[n + m] = '\n';
	m_buf[n + m + 1] = '\0';
	log_str = m_buf;

	m_mutex.unlock();

	if (this->is_async && !m_log_queue->full())
	{
		m_log_queue->push(log_str);
	}
	else
	{
		m_mutex.lock();
		fputs(log_str.c_str(), m_file);
		m_mutex.unlock();
	}

	va_end(valst);

}
void Log::flush()
{
	m_mutex.lock();
	//强制刷新写入流缓冲区
	fflush(m_file);
	m_mutex.unlock();
}
Log::~Log()
{
	delete[]m_log_queue;
	if (m_file != NULL)
	{
		fclose(m_file);
	}
}
