#include "sql_connection_pool.h"

// 初始化
void connection_pool::init(std::string url, std::string username, std::string password, std::string db_name, int port, int max_conn, int close_log)
{
	this->url = url;
	this->username = username;
	this->password = password;
	this->db_name = db_name;
	this->port = port;
	this->m_close_log = close_log;
	this->m_free_connect = 0;

	// 初始化连接池conn_pool
	for (int i = 0; i < max_conn; i++)
	{
		MYSQL* conn = NULL;
		// 初始化一个sql连接
		conn = mysql_init(conn);
		// 判断是否成功
		if (conn == NULL)
		{
			LOG_ERROR("MySQL Error init");
			exit(1);
		}
		// 初始化成功连接到数据库创建连接
		conn = mysql_real_connect(conn, url.c_str(), username.c_str(), password.c_str(), db_name.c_str(), port, NULL, 0);
		if (conn == NULL)
		{
			LOG_ERROR("MySQL Error connect");
			exit(1);
		}
		// 连接成功加入连接池
		this->conn_pool.push_back(conn);
		// 添加个数
		this->m_free_connect++;
	}
	cout << "add sem" << endl;
	// 设置信号量
	this->reserve = Sem(m_free_connect);
	// 最大个数
	this->max_conn = m_free_connect;
}

// 获取数据库连接
MYSQL* connection_pool::get_connection()
{
	MYSQL* conn = NULL;
	if (this->conn_pool.size() == 0)
	{
		return NULL;
	}
	// 锁定信号量
	this->reserve.wait();
	// 加锁
	lock.lock();
	// 获取连接
	conn = this->conn_pool.front();
	this->conn_pool.pop_front();
	// 设置信号
	--m_free_connect;
	++m_cur_connect;
	// 解锁
	lock.unlock();
	return conn;
}
// 释放一个连接
bool connection_pool::release_connection(MYSQL* conn)
{
	if (conn == NULL)
	{
		return false;
	}
	// 加锁
	lock.lock();
	this->conn_pool.push_back(conn);
	// 设置信号
	++m_free_connect;
	--m_cur_connect;
	// 解锁
	lock.unlock();
	// 释放信号量
	this->reserve.post();
	return true;
}
// 获取空闲的连接数
int connection_pool::get_free_connection()
{
	return this->m_free_connect;
}
// 销毁所有连接
bool connection_pool::destroy_pool()
{
	// 加锁
	lock.lock();
	if (this->conn_pool.size() > 0)
	{
		std::list<MYSQL*>::iterator it = this->conn_pool.begin();
		for (; it != this->conn_pool.end(); it++)
		{
			MYSQL* conn = (*it);
			// 关闭数据库连接
			mysql_close(conn);
		}
		m_free_connect = 0;
		m_cur_connect = 0;
		this->conn_pool.clear();
	}
	// 解锁
	lock.unlock();
	return true;
}


connection_pool::connection_pool() {}
connection_pool::~connection_pool()
{
	destroy_pool();
}


connectionRAII::connectionRAII(MYSQL** SQL, connection_pool* connPool)
{
	*SQL = connPool->get_connection();

	this->conRAII = *SQL;
	this->poolRAII = connPool;
}
connectionRAII::~connectionRAII()
{
	poolRAII->release_connection(conRAII);
}