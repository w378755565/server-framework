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

// 客户端连接信息
struct client_data
{
	// 客户端连接地址
	sockaddr_in address;
	// 客户端连接描述符
	int sockfd;
};

// 工具类
class server_util
{
public:
	// 管道
	static int* pipe_fd;
public:
	// 添加监听描述符到epoll树
	void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);
	// 设置非阻塞
	int setnonblocking(int fd);
	// 添加信号
	void addsig(int sig, void (handler)(int), bool restart = true);
	// 信号处理函数
	static void signal_handler(int sig);
};

