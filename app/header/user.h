#pragma once
#include "abshandler.h"

class user : public abs_handler
{
private:
	// ��������
	void do_request();
	// �û���¼
	void user_login();
	// �û�ע��
	void user_register();
};
