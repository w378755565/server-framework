#pragma once

#include <list>
#include <cstdio>
#include <iostream>
#include <exception>
#include <pthread.h>
#include "locker.h"

/* �̳߳أ�ģ��Ϊ���������� */
template<typename T>
class thread_pool
{
public:
	thread_pool(int actor_model, int m_max_task = 10000, int m_max_thread = 8);
	~thread_pool();

	// �������
	bool append(T* request, int state);
	// �������
	bool append(T* request);
private:
	// �ص�����
	static void* worker(void* arg);
	// ����ִ�е��߼�����
	void run();
	// ���������
	int m_max_task;
	// ����߳���
	int m_max_thread;
	// �̳߳�����
	pthread_t* m_threads;
	// �������
	std::list<T*> m_task_queue;
	// ģ��
	int actor_model;

	// �ź�����
	Sem m_queuestat;
	// ��
	Lock m_queuelock;
};

template<typename T>
thread_pool<T>::thread_pool(int actor_model, int m_max_task, int m_max_thread) :actor_model(actor_model), m_max_task(m_max_task), m_max_thread(m_max_thread), m_threads(NULL)
{
	if (m_max_task <= 0 || m_max_thread <= 0)
		throw std::exception();

	// ��������pthread_t
	this->m_threads = new pthread_t[m_max_thread];
	if (!m_threads)
		throw std::exception();

	// ��ʼ���߳�
	for (int i = 0; i < m_max_thread; i++)
	{
		if (pthread_create(this->m_threads + i, NULL, worker, this) != 0)
		{
			delete[] m_threads;
			throw std::exception();
		}
		// �����ɹ����÷���״̬
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

// �������
template<typename T>
bool thread_pool<T>::append(T* request, int functiontype)
{
	// ����
	m_queuelock.lock();
	if (m_task_queue.size() == m_max_task)
	{
		m_queuelock.unlock();
		return false;
	}
	request->functiontype = functiontype;
	m_task_queue.push_back(request);
	// �ͷ���
	m_queuelock.unlock();
	// �ͷ��ź���
	m_queuestat.post();
	return true;
}
// �������
template<typename T>
bool thread_pool<T>::append(T* request)
{
	// ����
	m_queuelock.lock();
	if (m_task_queue.size() == m_max_task)
	{
		m_queuelock.unlock();
		return false;
	}
	m_task_queue.push_back(request);
	// �ͷ���
	m_queuelock.unlock();
	// �ͷ��ź���
	m_queuestat.post();
	return true;
}
template <typename T>
void* thread_pool<T>::worker(void* arg)
{
	// ����run����������Ϊ�Լ���ָ��
	thread_pool* p = (thread_pool*)arg;
	p->run();
	return p;
}

template <typename T>
void thread_pool<T>::run()
{
	// ����ѭ��
	while (true)
	{
		// �����ź���
		m_queuestat.wait();

		// ����
		m_queuelock.lock();

		// ���л�ȡ����
		if (m_task_queue.empty())
		{
			m_queuelock.unlock();
			continue;
		}
		// ��ȡ����
		T* request = m_task_queue.front();
		m_task_queue.pop_front();
		// �ͷ���
		m_queuelock.unlock();

		// �������󣬸��ݴ���ģʽ����
		if (!request)
			continue;
		// �ж�ģʽ
		if (1 == actor_model)
		{
			// reactor����δ��ȡ
			if (0 == request->functiontype)
			{
				// ��ȡ
				if (request->read_once())
				{
					// ����improv
					request->improv = 1;
					request->process();
				}
				else
				{
					// ��ȡʧ��
					request->improv = 1;
				}
			}
			else
			{
				// д��
				if (request->write())
				{
					request->improv = 1;
				}
				else
				{
					// д��ʧ��
					request->improv = 1;
				}
			}
		}
		else
		{
			std::cout << "proactor" << std::endl;
			// proctorֻ��Ҫ�����ȡ���ݣ�д���ݲ���Ҫ����
			request->process();
		}

	}
}