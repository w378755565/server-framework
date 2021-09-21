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
#include <map>
#include <iostream>
#include "abshandler.h"

using namespace std;

/*
协议规定：
handler：应用层处理器是谁model,function
length：内容长度
分号后面为内容参数，冒号和分号为两个连续的时候表示转义
handler:1,2;length:100;username:1234;password:qwk::d;;;
*/


// 处理客户端连接，进行读取或者发送的消息管理
class client_conn
{
public:
	// 读取缓存大小
	static const int READ_BUFFER_SIZE = 2048;
	// 写入缓存大小
	static const int WRITE_BUFFER_SIZE = 1024;

	// 应用层的
	static map<int, abs_handler*>* app_handler;

	// 当前检测的项目，根据协议而定
	enum CHECK_ITEM
	{
		CHECK_ITEM_HANDLER,
		CHECK_ITEM_LENGTH,
		CHECK_ITEM_CONTENT
	};

	// 当前检测的项目状态
	enum ITEM_STATUS
	{
		// 成功
		ITEM_OK = 0,
		// 错误
		ITEM_BAD,
		// 数据未读完，需要继续获取
		ITEM_OPEN
	};

	// 协议状态码
	enum PROTOCOL_CODE
	{
		// 成功
		CODE_OK,
		// 错误
		CODE_BAD,
		// 内容数据未接收完
		CODE_NO
	};

public:
	// 外部添加监听的init
	void init(int sockfd, const sockaddr_in& addr);
	// 读取数据
	bool read_once();
	// 写数据函数
	bool write();
	// 处理读取到的数据函数
	void process();
private:
	// 初始化类内部成员用于长连接
	void init();
	// 解析读取数据
	PROTOCOL_CODE process_read();
	// 解析一项的内容
	ITEM_STATUS parse_item();
	// 解析协议的处理器
	PROTOCOL_CODE parse_handler(char* text);
	// 解析内容长度
	PROTOCOL_CODE parse_length(char* text);
	// 解析内容
	PROTOCOL_CODE parse_content(char* text);
public:
	// 用户连接任务时读还是写
	int functiontype;
	// 操作是否结束
	int improv;
	// epoll的根
	static int epoll_root;
	// 当前客户端连接的个数
	static int client_count;
private:
	// 当前客户端连接的文件描述符
	int sockfd;
	// 用户地址
	sockaddr_in address;
	// 模式ET还是LT
	int TRIGMode;
	// 读取的缓冲区
	char m_read_buf[READ_BUFFER_SIZE];
	// 当前读取的下标
	int m_read_idx;
	// 当前检测的下标
	int m_checked_idx;
	// 下一个开始的位置
	int start_item_idx;
	// 检测的项目
	CHECK_ITEM check_item;
	// 长度
	int length;
	// 模块
	int model;
	// 功能
	int function;
	// 参数
	map<string, string> params;
};
