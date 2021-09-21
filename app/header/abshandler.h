#pragma once

#include <string>
#include <map>
#include "locker.h"
#include "block_queue.h"

// ��������
class abs_handler
{
protected:
	// ��ǰ���ܱ��
	int function;
	// �����������
	std::map<std::string, std::string> params;
protected:
	virtual void do_request() = 0;
public:
	// ���캯��
	abs_handler();
	// ������������
	virtual ~abs_handler();
	// �����߼�
	void process(int function, std::map<std::string, std::string> params);
};


