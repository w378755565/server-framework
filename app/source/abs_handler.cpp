#include "abshandler.h"

abs_handler::abs_handler()
{
}

abs_handler::~abs_handler()
{
}

void abs_handler::process(int function, std::map<std::string, std::string> params)
{
	this->function = function;
	this->params = params;
	do_request();
}