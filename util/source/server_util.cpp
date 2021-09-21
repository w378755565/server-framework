#include "server_util.h"

void server_util::addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
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

// ���÷�����
int server_util::setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	// �µ��ļ�ѡ�� ���÷�����
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, new_option);
	return old_option;
}
// ����ź�
void server_util::addsig(int sig, void (handler)(int), bool restart)
{
	struct sigaction sa;
	// bzeroҲ����
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = handler;
	// ��ֹ�ź��жϺ�ϵͳ�쳣�����ж��ź����˺����������
	if (restart)
		sa.sa_flags |= SA_RESTART;
	sigfillset(&sa.sa_mask);
	// �����ź�
	assert(sigaction(sig, &sa, NULL) != -1);
}
// �źŴ�����
void server_util::signal_handler(int sig)
{
	// ����ԭ����errno
	int save_errno = errno;
	int msg = sig;
	// ���ܵ�������Ϣ
	send(pipe_fd[1], (char*)&msg, 1, 0);
	errno = save_errno;
}

int* server_util::pipe_fd = 0;