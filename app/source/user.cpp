#include "user.h"

// 处理请求
void user::do_request()
{
	switch (function)
	{
	case 1:
		user_login();
		break;
	case 2:
		user_register();
		break;
	default:
		cout << "unset this function" << endl;
	}
}

// 用户登录
void user::user_login()
{
	cout << "user login, login info:" << endl;
	cout << "username:" << params["username"] << endl;
	cout << "password:" << params["password"] << endl;
}

// 用户注册
void user::user_register()
{
	cout << "user register, register info:" << endl;
	cout << "username:" << params["username"] << endl;
	cout << "password:" << params["password"] << endl;
}

