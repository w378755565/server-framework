#include "user.h"

// ��������
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

// �û���¼
void user::user_login()
{
	cout << "user login, login info:" << endl;
	cout << "username:" << params["username"] << endl;
	cout << "password:" << params["password"] << endl;
}

// �û�ע��
void user::user_register()
{
	cout << "user register, register info:" << endl;
	cout << "username:" << params["username"] << endl;
	cout << "password:" << params["password"] << endl;
}

