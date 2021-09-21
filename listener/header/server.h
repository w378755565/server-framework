#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include <string.h>
#include <csignal>
#include "server_util.h"
#include "client_conn.h"
#include "thread_pool.h"
#include "log.h"
#include "app_header.h"

/*
������������ѭ��
*/

//����ļ����������ͻ������������
const int MAX_FD = 65536;

//����¼���
const int MAX_EVENT_NUMBER = 10000;

class server
{
private:
	int port;
	int m_close_log;
	int thread_poll_num;
	// ģ��
	int actormodel;
	// ����ģʽ
	int LISTENTrigmode;
public:
	// ���캯��
	server();
	// ��������
	~server();

	// ��ʼ������
	void init(int port, bool close_log, int thread_poll_num);
	// ѭ������
	void start();
	// ������Ϣ
	void send();
private:
	// ��ʼ��Ӧ�ò�Ĵ�����
	void init_app_handler();
	// ������
	void server_listen();
	// ������
	server_util util;
	// �������ļ�������
	int listenfd;
	// �źŹܵ�
	int pipefd[2];
	// �����ĸ�
	int epoll_root;
	// �¼�����
	epoll_event events[MAX_EVENT_NUMBER];
	// �̳߳�
	thread_pool<client_conn>* _pool;
private:
	// ����ͻ������ӽ���
	bool deal_client();
	// �����ź�
	bool deal_signal(bool &stopserver);
	// ����ͻ��˷��͵�����
	void deal_read(int sockfd);
	// �����͸��ͻ�������
	void deal_write(int sockfd);
	// ��ӿͻ�������
	void add_client(int connfd, sockaddr_in client_address);
	// �ͻ���������Ϣ
	client_data* clients;
	// �ͻ������Ӵ����б�
	client_conn* conns;
};

