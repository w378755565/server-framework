#include "server_config.h"

// �����˿�
int server_config::port = 8999;

// ���ݿ�����
char server_config::db_name[50] = "server";

// ���ݿ��¼�û���
char server_config::db_username[50] = "root";

// ���ݿ�����
char server_config::db_password[50] = "root";

// �Ƿ�ر���־����
bool server_config::close_log = false;

// ���ݿ�ظ���
int server_config::db_poll_num = 1000;

// �̳߳ظ���
int server_config::thread_poll_num = 500;
