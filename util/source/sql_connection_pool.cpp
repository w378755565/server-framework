#include "sql_connection_pool.h"

// ��ʼ��
void connection_pool::init(std::string url, std::string username, std::string password, std::string db_name, int port, int max_conn, int close_log)
{
	this->url = url;
	this->username = username;
	this->password = password;
	this->db_name = db_name;
	this->port = port;
	this->m_close_log = close_log;
	this->m_free_connect = 0;

	// ��ʼ�����ӳ�conn_pool
	for (int i = 0; i < max_conn; i++)
	{
		MYSQL* conn = NULL;
		// ��ʼ��һ��sql����
		conn = mysql_init(conn);
		// �ж��Ƿ�ɹ�
		if (conn == NULL)
		{
			LOG_ERROR("MySQL Error init");
			exit(1);
		}
		// ��ʼ���ɹ����ӵ����ݿⴴ������
		conn = mysql_real_connect(conn, url.c_str(), username.c_str(), password.c_str(), db_name.c_str(), port, NULL, 0);
		if (conn == NULL)
		{
			LOG_ERROR("MySQL Error connect");
			exit(1);
		}
		// ���ӳɹ��������ӳ�
		this->conn_pool.push_back(conn);
		// ��Ӹ���
		this->m_free_connect++;
	}
	cout << "add sem" << endl;
	// �����ź���
	this->reserve = Sem(m_free_connect);
	// ������
	this->max_conn = m_free_connect;
}

// ��ȡ���ݿ�����
MYSQL* connection_pool::get_connection()
{
	MYSQL* conn = NULL;
	if (this->conn_pool.size() == 0)
	{
		return NULL;
	}
	// �����ź���
	this->reserve.wait();
	// ����
	lock.lock();
	// ��ȡ����
	conn = this->conn_pool.front();
	this->conn_pool.pop_front();
	// �����ź�
	--m_free_connect;
	++m_cur_connect;
	// ����
	lock.unlock();
	return conn;
}
// �ͷ�һ������
bool connection_pool::release_connection(MYSQL* conn)
{
	if (conn == NULL)
	{
		return false;
	}
	// ����
	lock.lock();
	this->conn_pool.push_back(conn);
	// �����ź�
	++m_free_connect;
	--m_cur_connect;
	// ����
	lock.unlock();
	// �ͷ��ź���
	this->reserve.post();
	return true;
}
// ��ȡ���е�������
int connection_pool::get_free_connection()
{
	return this->m_free_connect;
}
// ������������
bool connection_pool::destroy_pool()
{
	// ����
	lock.lock();
	if (this->conn_pool.size() > 0)
	{
		std::list<MYSQL*>::iterator it = this->conn_pool.begin();
		for (; it != this->conn_pool.end(); it++)
		{
			MYSQL* conn = (*it);
			// �ر����ݿ�����
			mysql_close(conn);
		}
		m_free_connect = 0;
		m_cur_connect = 0;
		this->conn_pool.clear();
	}
	// ����
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