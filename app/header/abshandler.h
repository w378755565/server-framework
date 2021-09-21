#pragma once

#include <string>
#include <map>
#include "locker.h"
#include "block_queue.h"

// 抽象处理类
class abs_handler
{
protected:
	// 当前功能编号
	int function;
	// 参数处理队列
	std::map<std::string, std::string> params;
protected:
	virtual void do_request() = 0;
public:
	// 构造函数
	abs_handler();
	// 抽象析构函数
	virtual ~abs_handler();
	// 处理逻辑
	void process(int function, std::map<std::string, std::string> params);
};


