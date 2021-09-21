#pragma once

#include <list>
#include <string>
#include "mysql/mysql.h"
#include "locker.h"
#include "log.h"

class connection_pool
{
public:
	// 获取单例的连接池对象
	static connection_pool* get_instance()
	{
		static connection_pool pool;
		return &pool;
	}
	// 获取数据库连接
	MYSQL* get_connection();
	// 释放一个连接
	bool release_connection(MYSQL* conn);
	// 获取空闲的连接数
	int get_free_connection();
	// 销毁所有连接
	bool destroy_pool();
	// 初始化
	void init(std::string url, std::string username, std::string password, std::string db_name, int port, int max_conn, int close_log);
private:
	connection_pool();
	~connection_pool();

	// 数据库连接信息
	std::string url;
	std::string username;
	std::string password;
	std::string db_name;
	int port;
	int max_conn;

	// 日志开关
	int m_close_log;
	// 最大连接数
	int m_max_connect;
	// 当前已使用的连接数
	int m_cur_connect;
	// 当前剩余连接数
	int m_free_connect;
	// 连接池
	std::list<MYSQL*> conn_pool;
	// 锁
	Lock lock;
	// 信号量
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
