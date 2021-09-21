#include "server_config.h"

// 监听端口
int server_config::port = 8999;

// 数据库名称
char server_config::db_name[50] = "server";

// 数据库登录用户名
char server_config::db_username[50] = "root";

// 数据库密码
char server_config::db_password[50] = "root";

// 是否关闭日志功能
bool server_config::close_log = false;

// 数据库池个数
int server_config::db_poll_num = 1000;

// 线程池个数
int server_config::thread_poll_num = 500;
