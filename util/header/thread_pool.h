#pragma once

#include <list>
#include <cstdio>
#include <iostream>
#include <exception>
#include <pthread.h>
#include "locker.h"

/* 线程池，模板为请求任务类 */
template<typename T>
class thread_pool
{
public:
	thread_pool(int actor_model, int m_max_task = 10000, int m_max_thread = 8);
	~thread_pool();

	// 添加任务
	bool append(T* request, int state);
	// 添加任务
	bool append(T* request);
private:
	// 回调函数
	static void* worker(void* arg);
	// 具体执行的逻辑函数
	void run();
	// 最大任务数
	int m_max_task;
	// 最大线程数
	int m_max_thread;
	// 线程池数组
	pthread_t* m_threads;
	// 请求队列
	std::list<T*> m_task_queue;
	// 模型
	int actor_model;

	// 信号量锁
	Sem m_queuestat;
	// 锁
	Lock m_queuelock;
};

template<typename T>
thread_pool<T>::thread_pool(int actor_model, int m_max_task, int m_max_thread) :actor_model(actor_model), m_max_task(m_max_task), m_max_thread(m_max_thread), m_threads(NULL)
{
	if (m_max_task <= 0 || m_max_thread <= 0)
		throw std::exception();

	// 创建数组pthread_t
	this->m_threads = new pthread_t[m_max_thread];
	if (!m_threads)
		throw std::exception();

	// 初始化线程
	for (int i = 0; i < m_max_thread; i++)
	{
		if (pthread_create(this->m_threads + i, NULL, worker, this) != 0)
		{
			delete[] m_threads;
			throw std::exception();
		}
		// 创建成功设置分离状态
		if (pthread_detach(*(m_threads + i)) != 0)
		{
			delete[] m_threads;
			throw std::exception();
		}
	}
}

template<typename T>
thread_pool<T>::~thread_pool()
{
	delete[]m_threads;
}

// 添加任务
template<typename T>
bool thread_pool<T>::append(T* request, int functiontype)
{
	// 加锁
	m_queuelock.lock();
	if (m_task_queue.size() == m_max_task)
	{
		m_queuelock.unlock();
		return false;
	}
	request->functiontype = functiontype;
	m_task_queue.push_back(request);
	// 释放锁
	m_queuelock.unlock();
	// 释放信号量
	m_queuestat.post();
	return true;
}
// 添加任务
template<typename T>
bool thread_pool<T>::append(T* request)
{
	// 加锁
	m_queuelock.lock();
	if (m_task_queue.size() == m_max_task)
	{
		m_queuelock.unlock();
		return false;
	}
	m_task_queue.push_back(request);
	// 释放锁
	m_queuelock.unlock();
	// 释放信号量
	m_queuestat.post();
	return true;
}
template <typename T>
void* thread_pool<T>::worker(void* arg)
{
	// 运行run函数，参数为自己的指针
	thread_pool* p = (thread_pool*)arg;
	p->run();
	return p;
}

template <typename T>
void thread_pool<T>::run()
{
	// 无限循环
	while (true)
	{
		// 锁定信号量
		m_queuestat.wait();

		// 加锁
		m_queuelock.lock();

		// 队列获取任务
		if (m_task_queue.empty())
		{
			m_queuelock.unlock();
			continue;
		}
		// 获取任务
		T* request = m_task_queue.front();
		m_task_queue.pop_front();
		// 释放锁
		m_queuelock.unlock();

		// 处理请求，根据处理模式进行
		if (!request)
			continue;
		// 判断模式
		if (1 == actor_model)
		{
			// reactor，还未读取
			if (0 == request->functiontype)
			{
				// 读取
				if (request->read_once())
				{
					// 设置improv
					request->improv = 1;
					request->process();
				}
				else
				{
					// 读取失败
					request->improv = 1;
				}
			}
			else
			{
				// 写入
				if (request->write())
				{
					request->improv = 1;
				}
				else
				{
					// 写入失败
					request->improv = 1;
				}
			}
		}
		else
		{
			std::cout << "proactor" << std::endl;
			// proctor只需要处理读取数据，写数据不需要处理
			request->process();
		}

	}
}