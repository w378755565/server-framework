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

// ������Ϣ
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
	// ��Ӵ�����
	client_conn::app_handler->insert(pair<int, abs_handler*>(1, new user()));

}

/// <summary>
/// ����������epoll��
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

	// �˿ڸ���
	int flag = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

	// ��
	ret = bind(listenfd, (sockaddr*)&address, sizeof(address));
	assert(ret >= 0);

	// ����
	ret = listen(listenfd, 5);
	assert(ret >= 0);

	// ������Ҫ��fd��Ϣ
	server_util::pipe_fd = pipefd;

	// epoll��ʼ��
	epoll_root = epoll_create(5);
	assert(epoll_root != -1);

	// ���listenfd��epoll����
	util.addfd(epoll_root, listenfd, false, 1);

	// ����epoll���ͻ������Ӷ���
	client_conn::epoll_root = epoll_root;

	// ���ùܵ�Ϊȫ˫��
	ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);

	// ���ö�ȡ�˷�����
	util.setnonblocking(pipefd[1]);

	// ���pipe�ܵ�������
	util.addfd(epoll_root, pipefd[0], false, 1);

	// ���԰�ر�״̬�ź�
	util.addsig(SIGPIPE, SIG_IGN);

	// ���ùر��ź�
	util.addsig(SIGTERM, util.signal_handler, false);

	// ��������ʱ���Է��Ϳ�����������...

	cout << "listen port:" << address.sin_port << endl;
	cout << "epoll root:" << epoll_root << endl;
}

void server::start()
{
	// ����������
	bool stop_server = false;

	cout << "start loop:" << endl;
	while (!stop_server)
	{
		// epoll_wait
		int number = epoll_wait(epoll_root, events, MAX_EVENT_NUMBER, -1);

		// �����������жϸ���
		if (number < 0 && errno != EINTR)
		{
			// ����
			// LOG_ERROR("%s", "epoll failure");
			break;
		}

		// ��������
		for (int i = 0; i < number; i++)
		{
			// �õ����������ļ�������
			int sockfd = events[i].data.fd;

			// �Ƿ��ǿͻ������ӽ�����
			if (sockfd == listenfd)
			{
				cout << "client connect" << endl;
				// �ͻ������ӽ���
				bool flag = deal_client();
				if (false == flag)
					continue;
				cout << "client link ok" << endl;
			}
			else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
			{
				// �������ر�����
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
				// ������ź�EPOLLPIPT�����д���
				cout << "stop_server:" << stop_server << endl;
				bool flag = deal_signal(stop_server);
				/*if (false == flag)
					LOG_ERROR("%s", "dealclientdata failure");*/
			}
			else if (events[i].events & EPOLLIN)
			{
				// �յ��ͻ��˷�����������
				cout << "read data:" << stop_server << endl;
				deal_read(sockfd);
			}
			else if (events[i].events & EPOLLOUT)
			{
				// д�¼�������������ֵ�����ɴ������Ѿ������ݶ�ȡ���������ˣ���ʱֻ��Ҫ�����ݷ��ͼ���
				deal_write(sockfd);
			}


		}
	}
}

// ����ͻ������ӽ���
bool server::deal_client()
{
	// �ͻ������ӵ�ַ
	sockaddr_in client_address;
	socklen_t client_addrlength = sizeof(client_address);

	if (0 == LISTENTrigmode)
	{
		cout << "accept data" << endl;
		// FTģʽ��һ�ζ�ȡ���
		int connfd = accept(listenfd, (sockaddr*)&client_address, &client_addrlength);
		cout << "connfd:" << connfd << endl;
		if (connfd < 0)
		{
			LOG_ERROR("%s:errno is:%d", "accept error", errno);
			return false;
		}
		// ����ͻ������Ӹ���̫�࣬������ʧ��
		if (client_conn::client_count >= MAX_FD)
		{
			// �ͻ������Ӵ�����Ҫ���ͻ��˷���һ��������Ϣ��
			// TODO...
			LOG_ERROR("%s", "Internal server busy");
			return false;
		}
		// ��ӿͻ�����Ϣ
		add_client(connfd, client_address);
	}
	else
	{
		// ETģʽ��Ҫѭ����ȡ��ֻ�з��͵�ʱ����ܶ�ȡ
		while (true)
		{
			int connfd = accept(listenfd, (sockaddr*)&client_address, &client_addrlength);
			if (connfd < 0)
			{
				LOG_ERROR("%s:errno is:%d", "accept error", errno);
				return false;
			}
			// ����ͻ������Ӹ���̫�࣬������ʧ��
			if (client_conn::client_count >= MAX_FD)
			{
				// �ͻ������Ӵ�����Ҫ���ͻ��˷���һ��������Ϣ��
				// TODO...
				LOG_ERROR("%s", "Internal server busy");
				return false;
			}
			// ��ӿͻ�����Ϣ
			add_client(connfd, client_address);
		}
		return false;
	}
	return true;
}

// �����ź�
bool server::deal_signal(bool& stopserver)
{
	char signals[1024];
	// ��ȡ�źţ������ALRM���ʾ��������Ҫ�޳���ʱ�ͻ��ˣ�����ΪTERMʱ����ʾ��ֹ�źţ���ʱ�������ر�
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
			// �����ź�
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

// ����ͻ��˷��͵�����
void server::deal_read(int sockfd)
{
	// reactor
	if (1 == actormodel)
	{
		// ��������������
		_pool->append(conns + sockfd);
		while (true)
		{
			// �ж��Ƿ�ɹ�
			if (1 == conns[sockfd].improv)
			{
				conns[sockfd].improv = 0;
				break;
			}
		}
	}
	else if (2 == actormodel)
	{
		// proactor��ȡ����
		if (conns[sockfd].read_once())
		{
			// ��ȡ�ɹ���������������
			_pool->append(conns + sockfd);
		}
		else
		{
			// ��ȡ���󣬹رտͻ���
			epoll_ctl(epoll_root, EPOLL_CTL_DEL, sockfd, NULL);
			close(sockfd);
			client_conn::client_count--;
		}
	}
}

// �����͸��ͻ�������
void server::deal_write(int sockfd)
{
	// reactor
	if (1 == actormodel)
	{
		// ��������������
		_pool->append(conns + sockfd, 1);
		while (true)
		{
			// �ж��Ƿ�ɹ�
			if (1 == conns[sockfd].improv)
			{
				conns[sockfd].improv = 0;
				break;
			}
		}
	}
	else if (2 == actormodel)
	{
		// proactor��ȡ����
		if (conns[sockfd].write())
		{
			LOG_INFO("send data to the client");
		}
		else
		{
			// ��ȡ���󣬹رտͻ���
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