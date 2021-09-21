#include "server.h"
#include "server_config.h"

int main()
{
	server ser;

	ser.init(server_config::port, server_config::close_log, server_config::thread_poll_num);

	ser.start();

	return 0;
}
