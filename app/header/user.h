#pragma once
#include "abshandler.h"

class user : public abs_handler
{
private:
	// 处理请求
	void do_request();
	// 用户登录
	void user_login();
	// 用户注册
	void user_register();
};
