#pragma once

// ������

class server_config
{
public:
	// �����˿�
	static int port;
	// ���ݿ�����
	static char db_name[50];
	// ���ݿ��¼�û���
	static char db_username[50];
	// ���ݿ�����
	static char db_password[50];
	// �Ƿ�ر���־����
	static bool close_log;
	// ���ݿ�ظ���
	static int db_poll_num;
	// �̳߳ظ���
	static int thread_poll_num;
};

