#pragma once

#include <list>
#include <string>
#include "mysql/mysql.h"
#include "locker.h"
#include "log.h"

class connection_pool
{
public:
	// ��ȡ���������ӳض���
	static connection_pool* get_instance()
	{
		static connection_pool pool;
		return &pool;
	}
	// ��ȡ���ݿ�����
	MYSQL* get_connection();
	// �ͷ�һ������
	bool release_connection(MYSQL* conn);
	// ��ȡ���е�������
	int get_free_connection();
	// ������������
	bool destroy_pool();
	// ��ʼ��
	void init(std::string url, std::string username, std::string password, std::string db_name, int port, int max_conn, int close_log);
private:
	connection_pool();
	~connection_pool();

	// ���ݿ�������Ϣ
	std::string url;
	std::string username;
	std::string password;
	std::string db_name;
	int port;
	int max_conn;

	// ��־����
	int m_close_log;
	// ���������
	int m_max_connect;
	// ��ǰ��ʹ�õ�������
	int m_cur_connect;
	// ��ǰʣ��������
	int m_free_connect;
	// ���ӳ�
	std::list<MYSQL*> conn_pool;
	// ��
	Lock lock;
	// �ź���
	Sem reserve;
};

class connectionRAII
{
public:
	connectionRAII(MYSQL** SQL, connection_pool* connPool);
	~connectionRAII();
private:
	MYSQL* conRAII;
	connection_pool* poolRAII;
};
