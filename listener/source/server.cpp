#include "server.h"

server::server()
{
	clients = new client_data[MAX_FD];
	conns = new client_conn[MAX_FD];
	LISTENTrigmode = 0;
	actormodel = 1;
}
server::~server()
{
	delete[] clients;
	delete[] conns;
	delete _pool;
}

// 发送消息
void server::send()
{

}

void server::init(int port, bool close_log, int thread_poll_num)
{
	this->port = port;
	this->m_close_log = close_log;
	this->thread_poll_num = thread_poll_num;

	this->_pool = new thread_pool<client_conn>(actormodel, thread_poll_num);
	init_app_handler();
	server_listen();
}

void server::init_app_handler()
{
	client_conn::app_handler = new map<int, abs_handler*>();
	// 添加处理器
	client_conn::app_handler->insert(pair<int, abs_handler*>(1, new user()));

}

/// <summary>
/// 开启监听，epoll树
/// </summary>
void server::server_listen()
{
	listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);

	int ret = 0;
	sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	// 端口复用
	int flag = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

	// 绑定
	ret = bind(listenfd, (sockaddr*)&address, sizeof(address));
	assert(ret >= 0);

	// 监听
	ret = listen(listenfd, 5);
	assert(ret >= 0);

	// 设置需要的fd信息
	server_util::pipe_fd = pipefd;

	// epoll初始化
	epoll_root = epoll_create(5);
	assert(epoll_root != -1);

	// 添加listenfd到epoll树中
	util.addfd(epoll_root, listenfd, false, 1);

	// 设置epoll到客户端连接对象
	client_conn::epoll_root = epoll_root;

	// 设置管道为全双工
	ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);

	// 设置读取端非阻塞
	util.setnonblocking(pipefd[1]);

	// 添加pipe管道到监听
	util.addfd(epoll_root, pipefd[0], false, 1);

	// 忽略半关闭状态信号
	util.addsig(SIGPIPE, SIG_IGN);

	// 设置关闭信号
	util.addsig(SIGTERM, util.signal_handler, false);

	// 结束，此时可以发送开启心跳机制...

	cout << "listen port:" << address.sin_port << endl;
	cout << "epoll root:" << epoll_root << endl;
}

void server::start()
{
	// 服务器开关
	bool stop_server = false;

	cout << "start loop:" << endl;
	while (!stop_server)
	{
		// epoll_wait
		int number = epoll_wait(epoll_root, events, MAX_EVENT_NUMBER, -1);

		// 监听到数据判断个数
		if (number < 0 && errno != EINTR)
		{
			// 错误
			// LOG_ERROR("%s", "epoll failure");
			break;
		}

		// 监听到了
		for (int i = 0; i < number; i++)
		{
			// 得到监听到的文件描述符
			int sockfd = events[i].data.fd;

			// 是否是客户端连接进入了
			if (sockfd == listenfd)
			{
				cout << "client connect" << endl;
				// 客户端连接进入
				bool flag = deal_client();
				if (false == flag)
					continue;
				cout << "client link ok" << endl;
			}
			else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
			{
				// 服务器关闭连接
				cout << "client close" << endl;
				map<int, abs_handler*>::iterator it = client_conn::app_handler->begin();
				for (; it != client_conn::app_handler->end(); it++)
				{
					delete it->second;
				}
				delete client_conn::app_handler;
			}
			else if ((sockfd == pipefd[0]) && (events[i].events & EPOLLIN))
			{
				// 如果是信号EPOLLPIPT，进行处理
				cout << "stop_server:" << stop_server << endl;
				bool flag = deal_signal(stop_server);
				/*if (false == flag)
					LOG_ERROR("%s", "dealclientdata failure");*/
			}
			else if (events[i].events & EPOLLIN)
			{
				// 收到客户端发送来的数据
				cout << "read data:" << stop_server << endl;
				deal_read(sockfd);
			}
			else if (events[i].events & EPOLLOUT)
			{
				// 写事件，设置向量的值，即可触发，已经将数据读取到缓冲区了，此时只需要将数据发送即可
				deal_write(sockfd);
			}


		}
	}
}

// 处理客户端连接进入
bool server::deal_client()
{
	// 客户端连接地址
	sockaddr_in client_address;
	socklen_t client_addrlength = sizeof(client_address);

	if (0 == LISTENTrigmode)
	{
		cout << "accept data" << endl;
		// FT模式，一次读取完成
		int connfd = accept(listenfd, (sockaddr*)&client_address, &client_addrlength);
		cout << "connfd:" << connfd << endl;
		if (connfd < 0)
		{
			LOG_ERROR("%s:errno is:%d", "accept error", errno);
			return false;
		}
		// 如果客户端连接个数太多，则连接失败
		if (client_conn::client_count >= MAX_FD)
		{
			// 客户端连接错误，需要给客户端发送一个错误消息。
			// TODO...
			LOG_ERROR("%s", "Internal server busy");
			return false;
		}
		// 添加客户端信息
		add_client(connfd, client_address);
	}
	else
	{
		// ET模式需要循环读取，只有发送的时候才能读取
		while (true)
		{
			int connfd = accept(listenfd, (sockaddr*)&client_address, &client_addrlength);
			if (connfd < 0)
			{
				LOG_ERROR("%s:errno is:%d", "accept error", errno);
				return false;
			}
			// 如果客户端连接个数太多，则连接失败
			if (client_conn::client_count >= MAX_FD)
			{
				// 客户端连接错误，需要给客户端发送一个错误消息。
				// TODO...
				LOG_ERROR("%s", "Internal server busy");
				return false;
			}
			// 添加客户端信息
			add_client(connfd, client_address);
		}
		return false;
	}
	return true;
}

// 处理信号
bool server::deal_signal(bool& stopserver)
{
	char signals[1024];
	// 获取信号，如果是ALRM则表示正常，需要剔除超时客户端，否则为TERM时，表示终止信号，此时服务器关闭
	int ret = recv(pipefd[0], signals, sizeof(signals), 0);
	if (ret == -1)
	{
		return false;
	}
	else if (ret == 0)
	{
		return false;
	}
	else
	{
		for (int i = 0; i < ret; i++)
		{
			// 处理信号
			switch (signals[i])
			{
			case SIGTERM:
				stopserver = true;
				break;
			}
		}
	}
	return true;
}

// 处理客户端发送的数据
void server::deal_read(int sockfd)
{
	// reactor
	if (1 == actormodel)
	{
		// 交给处理器处理
		_pool->append(conns + sockfd);
		while (true)
		{
			// 判断是否成功
			if (1 == conns[sockfd].improv)
			{
				conns[sockfd].improv = 0;
				break;
			}
		}
	}
	else if (2 == actormodel)
	{
		// proactor读取数据
		if (conns[sockfd].read_once())
		{
			// 读取成功交给处理器处理
			_pool->append(conns + sockfd);
		}
		else
		{
			// 读取错误，关闭客户端
			epoll_ctl(epoll_root, EPOLL_CTL_DEL, sockfd, NULL);
			close(sockfd);
			client_conn::client_count--;
		}
	}
}

// 处理发送给客户端数据
void server::deal_write(int sockfd)
{
	// reactor
	if (1 == actormodel)
	{
		// 交给处理器处理
		_pool->append(conns + sockfd, 1);
		while (true)
		{
			// 判断是否成功
			if (1 == conns[sockfd].improv)
			{
				conns[sockfd].improv = 0;
				break;
			}
		}
	}
	else if (2 == actormodel)
	{
		// proactor读取数据
		if (conns[sockfd].write())
		{
			LOG_INFO("send data to the client");
		}
		else
		{
			// 读取错误，关闭客户端
			epoll_ctl(epoll_root, EPOLL_CTL_DEL, sockfd, NULL);
			close(sockfd);
			client_conn::client_count--;
		}
	}
}

void server::add_client(int connfd, sockaddr_in client_address)
{
	conns[connfd].init(connfd, client_address);
	clients[connfd].address = client_address;
	clients[connfd].sockfd = connfd;
}