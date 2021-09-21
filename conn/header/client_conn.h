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
Э��涨��
handler��Ӧ�ò㴦������˭model,function
length�����ݳ���
�ֺź���Ϊ���ݲ�����ð�źͷֺ�Ϊ����������ʱ���ʾת��
handler:1,2;length:100;username:1234;password:qwk::d;;;
*/


// ����ͻ������ӣ����ж�ȡ���߷��͵���Ϣ����
class client_conn
{
public:
	// ��ȡ�����С
	static const int READ_BUFFER_SIZE = 2048;
	// д�뻺���С
	static const int WRITE_BUFFER_SIZE = 1024;

	// Ӧ�ò��
	static map<int, abs_handler*>* app_handler;

	// ��ǰ������Ŀ������Э�����
	enum CHECK_ITEM
	{
		CHECK_ITEM_HANDLER,
		CHECK_ITEM_LENGTH,
		CHECK_ITEM_CONTENT
	};

	// ��ǰ������Ŀ״̬
	enum ITEM_STATUS
	{
		// �ɹ�
		ITEM_OK = 0,
		// ����
		ITEM_BAD,
		// ����δ���꣬��Ҫ������ȡ
		ITEM_OPEN
	};

	// Э��״̬��
	enum PROTOCOL_CODE
	{
		// �ɹ�
		CODE_OK,
		// ����
		CODE_BAD,
		// ��������δ������
		CODE_NO
	};

public:
	// �ⲿ��Ӽ�����init
	void init(int sockfd, const sockaddr_in& addr);
	// ��ȡ����
	bool read_once();
	// д���ݺ���
	bool write();
	// �����ȡ�������ݺ���
	void process();
private:
	// ��ʼ�����ڲ���Ա���ڳ�����
	void init();
	// ������ȡ����
	PROTOCOL_CODE process_read();
	// ����һ�������
	ITEM_STATUS parse_item();
	// ����Э��Ĵ�����
	PROTOCOL_CODE parse_handler(char* text);
	// �������ݳ���
	PROTOCOL_CODE parse_length(char* text);
	// ��������
	PROTOCOL_CODE parse_content(char* text);
public:
	// �û���������ʱ������д
	int functiontype;
	// �����Ƿ����
	int improv;
	// epoll�ĸ�
	static int epoll_root;
	// ��ǰ�ͻ������ӵĸ���
	static int client_count;
private:
	// ��ǰ�ͻ������ӵ��ļ�������
	int sockfd;
	// �û���ַ
	sockaddr_in address;
	// ģʽET����LT
	int TRIGMode;
	// ��ȡ�Ļ�����
	char m_read_buf[READ_BUFFER_SIZE];
	// ��ǰ��ȡ���±�
	int m_read_idx;
	// ��ǰ�����±�
	int m_checked_idx;
	// ��һ����ʼ��λ��
	int start_item_idx;
	// ������Ŀ
	CHECK_ITEM check_item;
	// ����
	int length;
	// ģ��
	int model;
	// ����
	int function;
	// ����
	map<string, string> params;
};
