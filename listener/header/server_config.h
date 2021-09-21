#pragma once

// 配置类

class server_config
{
public:
	// 监听端口
	static int port;
	// 数据库名称
	static char db_name[50];
	// 数据库登录用户名
	static char db_username[50];
	// 数据库密码
	static char db_password[50];
	// 是否关闭日志功能
	static bool close_log;
	// 数据库池个数
	static int db_poll_num;
	// 线程池个数
	static int thread_poll_num;
};

