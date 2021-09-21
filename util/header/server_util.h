#pragma once

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

// �ͻ���������Ϣ
struct client_data
{
	// �ͻ������ӵ�ַ
	sockaddr_in address;
	// �ͻ�������������
	int sockfd;
};

// ������
class server_util
{
public:
	// �ܵ�
	static int* pipe_fd;
public:
	// ��Ӽ�����������epoll��
	void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);
	// ���÷�����
	int setnonblocking(int fd);
	// ����ź�
	void addsig(int sig, void (handler)(int), bool restart = true);
	// �źŴ�����
	static void signal_handler(int sig);
};

