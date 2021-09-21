#include "client_conn.h"

int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	// �µ��ļ�ѡ�� ���÷�����
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, new_option);
	return old_option;
}

void addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
	// epoll�¼�
	epoll_event event;
	// ����fd
	event.data.fd = fd;

	// �Ƿ���ETģʽ
	if (1 == TRIGMode)
		// ����ETģʽ
		event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	else
		event.events = EPOLLIN | EPOLLRDHUP;

	// ������õ���ģʽ�������õ��Σ�ÿ��epoll_wait�󣬱���epoll_ctl��������
	if (one_shot)
		event.events |= EPOLLONESHOT;
	// ctl
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	// ���÷�����
	setnonblocking(fd);
}


/// <summary>
/// ����������oneshot�����ÿ���յ������epoll�������ֶ�ctrl���ܼ���ʹ��
/// </summary>
/// <param name="epollfd">��</param>
/// <param name="fd">������</param>
/// <param name="ev">�����¼�</param>
/// <param name="TRIGMode">ET����LT</param>
void modfd(int epollfd, int fd, int ev, int TRIGMode)
{
	epoll_event event;
	event.data.fd = fd;

	if (1 == TRIGMode)
		event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
	else
		event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

int client_conn::client_count = 0;
int client_conn::epoll_root = -1;
map<int, abs_handler*>* client_conn::app_handler = 0;

void client_conn::init(int sockfd, const sockaddr_in& addr)
{
	this->sockfd = sockfd;
	this->address = addr;
	// ����ļ��������ļ�����epoll
	addfd(epoll_root, sockfd, true, TRIGMode);

	client_count++;
}
void client_conn::init()
{
	m_read_idx = 0;
	m_checked_idx = 0;
	start_item_idx = 0;
	check_item = CHECK_ITEM_HANDLER;
}
// ��ȡ����
bool client_conn::read_once()
{
	// �����Ѿ����ˣ��������ټ�����ȡ
	if (m_read_idx >= READ_BUFFER_SIZE)
	{
		return false;
	}
	int bytes_read = 0;
	// LTģʽ
	if (0 == TRIGMode)
	{
		bytes_read = recv(sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
		m_read_idx += bytes_read;
		return bytes_read > 0;
	}
	else
	{
		// ETģʽ��Ҫѭ����ȡ
		while (true)
		{
			bytes_read = recv(sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
			if (bytes_read == -1)
			{
				// һ���ٴγ��Ի��������������������Ϊֻ��ȡһ��
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					break;
				return false;
			}
			else if (bytes_read == 0)
			{
				return false;
			}
			m_read_idx += bytes_read;
		}
		return true;
	}
}

// д���ݺ���
bool client_conn::write()
{
	return false;
}

// ��������״̬����ʽ��������
void client_conn::process()
{
	process_read();
	modfd(epoll_root, sockfd, EPOLLOUT, TRIGMode);

}

client_conn::PROTOCOL_CODE client_conn::process_read()
{
	ITEM_STATUS item_status = ITEM_OK;
	PROTOCOL_CODE ret;

	char* text = 0;
	// �����ǰ������ݣ�����״̬Ϊ��ȡ��ɵģ����ߵ�ǰһ�������Ϊ��ȡ��ɵ�
	while ((check_item == CHECK_ITEM_CONTENT && item_status == ITEM_OK) ||
		(item_status = parse_item()) == ITEM_OK)
	{
		text = m_read_buf + start_item_idx;
		start_item_idx = m_checked_idx;
		// ��������
		switch (check_item)
		{
		case CHECK_ITEM_HANDLER:
			ret = parse_handler(text);
			if (ret == CODE_BAD)
				return CODE_BAD;
			break;
		case CHECK_ITEM_LENGTH:
			ret = parse_length(text);
			if (ret == CODE_BAD)
				return CODE_BAD;
			break;
		case CHECK_ITEM_CONTENT:
			ret = parse_content(text);
			cout << "content parse over, ret:" << ret << endl;
			if (ret == CODE_BAD)
				return CODE_BAD;
			else if (ret == CODE_OK)
			{
				// ����Ӧ�ò㴦��
				map<int, abs_handler*>::iterator model_it = app_handler->find(model);
				model_it->second->process(function, params);
				return CODE_OK;
			}
			item_status = ITEM_OPEN;
			break;
		default:
			break;
		}
	}
}

client_conn::ITEM_STATUS client_conn::parse_item()
{
	char temp;
	for (; m_checked_idx < m_read_idx; m_checked_idx++)
	{
		temp = m_read_buf[m_checked_idx];
		if (temp == ';')
		{
			m_read_buf[m_checked_idx++] = '\0';
			return ITEM_OK;
		}
	}
	return ITEM_OPEN;
}

client_conn::PROTOCOL_CODE client_conn::parse_handler(char* text)
{
	char* text_arr = strtok(text, ":");
	text_arr = strtok(NULL, ":");
	text_arr = strtok(text_arr, ",");
	model = atoi(text_arr);
	text_arr = strtok(NULL, ",");
	function = atoi(text_arr);

	cout << "model:" << model << endl;
	cout << "function:" << function << endl;

	check_item = CHECK_ITEM_LENGTH;

	return CODE_OK;
}

client_conn::PROTOCOL_CODE client_conn::parse_length(char* text)
{
	char* text_arr = strtok(text, ":");
	text_arr = strtok(NULL, ":");
	length = atoi(text_arr);
	cout << "length:" << length << endl;

	check_item = CHECK_ITEM_CONTENT;

	return CODE_OK;
}

client_conn::PROTOCOL_CODE client_conn::parse_content(char* text)
{
	if (m_read_idx >= (length + m_checked_idx))
	{
		// ���ݽ������
		char* p = text;
		cout << "text:" << text << endl;
		string key;
		string value;
		bool iskey = true;
		char x = '\0';
		while (*p)
		{
			x = *p;
			if (*p == ':' && *(p + 1) == ':' || *p == ';' && *(p + 1) == ';')
			{
				// ת��ð��
				p += 2;
			}
			else if (*p == ':')
			{
				cout << "key:" << key << endl;
				// key��value�ķָ���
				iskey = false;
				p++;
				continue;
			}
			else if (*p == ';')
			{
				cout << "value:" << value << endl;
				params.insert(pair<string, string>(key, value));
				key.clear();
				value.clear();
				// �����ָ���
				iskey = true;
				p++;
				continue;
			}
			else
			{
				p++;
			}
			if (iskey)
			{
				key = key + x;
			}
			else
			{
				value = value + x;
			}
		}
		return CODE_OK;
	}
	return CODE_NO;
}

