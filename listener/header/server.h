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
开启服务器的循环
*/

//最大文件描述符，客户端最大连接数
const int MAX_FD = 65536;

//最大事件数
const int MAX_EVENT_NUMBER = 10000;

class server
{
private:
	int port;
	int m_close_log;
	int thread_poll_num;
	// 模型
	int actormodel;
	// 监听模式
	int LISTENTrigmode;
public:
	// 构造函数
	server();
	// 析构函数
	~server();

	// 初始化函数
	void init(int port, bool close_log, int thread_poll_num);
	// 循环监听
	void start();
	// 发送消息
	void send();
private:
	// 初始化应用层的处理器
	void init_app_handler();
	// 开启绑定
	void server_listen();
	// 工具类
	server_util util;
	// 监听的文件描述符
	int listenfd;
	// 信号管道
	int pipefd[2];
	// 监听的根
	int epoll_root;
	// 事件数组
	epoll_event events[MAX_EVENT_NUMBER];
	// 线程池
	thread_pool<client_conn>* _pool;
private:
	// 处理客户端连接进入
	bool deal_client();
	// 处理信号
	bool deal_signal(bool &stopserver);
	// 处理客户端发送的数据
	void deal_read(int sockfd);
	// 处理发送给客户端数据
	void deal_write(int sockfd);
	// 添加客户端连接
	void add_client(int connfd, sockaddr_in client_address);
	// 客户端连接信息
	client_data* clients;
	// 客户端连接处理列表
	client_conn* conns;
};

