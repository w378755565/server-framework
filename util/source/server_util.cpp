#include "server_util.h"

void server_util::addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
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

// 设置非阻塞
int server_util::setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	// 新的文件选择 设置非阻塞
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, new_option);
	return old_option;
}
// 添加信号
void server_util::addsig(int sig, void (handler)(int), bool restart)
{
	struct sigaction sa;
	// bzero也可以
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = handler;
	// 防止信号中断后系统异常，有中断信号来了后会重新启动
	if (restart)
		sa.sa_flags |= SA_RESTART;
	sigfillset(&sa.sa_mask);
	// 设置信号
	assert(sigaction(sig, &sa, NULL) != -1);
}
// 信号处理函数
void server_util::signal_handler(int sig)
{
	// 保留原来的errno
	int save_errno = errno;
	int msg = sig;
	// 给管道发送信息
	send(pipe_fd[1], (char*)&msg, 1, 0);
	errno = save_errno;
}

int* server_util::pipe_fd = 0;