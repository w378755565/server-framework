#include "client_conn.h"

int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	// 新的文件选择 设置非阻塞
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, new_option);
	return old_option;
}

void addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
	// epoll事件
	epoll_event event;
	// 设置fd
	event.data.fd = fd;

	// 是否开启ET模式
	if (1 == TRIGMode)
		// 开启ET模式
		event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	else
		event.events = EPOLLIN | EPOLLRDHUP;

	// 如果设置单次模式，则设置单次，每次epoll_wait后，必须epoll_ctl重新设置
	if (one_shot)
		event.events |= EPOLLONESHOT;
	// ctl
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	// 设置非阻塞
	setnonblocking(fd);
}


/// <summary>
/// 由于设置了oneshot，因此每次收到请求后，epoll都必须手动ctrl才能继续使用
/// </summary>
/// <param name="epollfd">根</param>
/// <param name="fd">描述符</param>
/// <param name="ev">基础事件</param>
/// <param name="TRIGMode">ET还是LT</param>
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
	// 添加文件描述符的监听到epoll
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
// 读取数据
bool client_conn::read_once()
{
	// 缓存已经满了，不可以再继续读取
	if (m_read_idx >= READ_BUFFER_SIZE)
	{
		return false;
	}
	int bytes_read = 0;
	// LT模式
	if (0 == TRIGMode)
	{
		bytes_read = recv(sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
		m_read_idx += bytes_read;
		return bytes_read > 0;
	}
	else
	{
		// ET模式需要循环读取
		while (true)
		{
			bytes_read = recv(sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
			if (bytes_read == -1)
			{
				// 一旦再次尝试或者阻塞就立马结束，因为只读取一次
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

// 写数据函数
bool client_conn::write()
{
	return false;
}

// 处理函数，状态机方式解析数据
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
	// 如果当前检测内容，并且状态为读取完成的，或者当前一项的数据为读取完成的
	while ((check_item == CHECK_ITEM_CONTENT && item_status == ITEM_OK) ||
		(item_status = parse_item()) == ITEM_OK)
	{
		text = m_read_buf + start_item_idx;
		start_item_idx = m_checked_idx;
		// 解析内容
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
				// 交给应用层处理
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
		// 内容接收完成
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
				// 转义冒号
				p += 2;
			}
			else if (*p == ':')
			{
				cout << "key:" << key << endl;
				// key和value的分隔符
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
				// 参数分隔符
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

