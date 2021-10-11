#include "XThread.h"
#include "XTask.h"
#include <thread>
#include <iostream>
#include <event2/event.h>
#ifdef _WIN32
#else
#include <unistd.h>
#endif

using namespace std;

//�����߳�����Ļص�����
static void NotifyCallback(evutil_socket_t fd, short which, void* arg)
{
	XThread* t = (XThread*)arg;
	t->Notify(fd, which);

}

void XThread::Notify(evutil_socket_t fd, short which)
{
	//ˮƽ������ֻҪû�н�����ȫ ���ٴν���
	char buf[2] = { 0 };
#ifdef _WIN32
	int re = recv(fd, buf, 1, 0);
#else
	int re = read(fd, buf, 1);
#endif
	if (re <= 0)
		return;
	cout << id << " thread " << buf << endl;
	XTask* task = nullptr;
	//��ȡ���� ����ʼ������
	tasks_mutex.lock();
	if (tasks.empty())
	{
		tasks_mutex.unlock();
		return;
	}
	task = tasks.front();  //�Ƚ��ȳ�
	tasks.pop_front();
	tasks_mutex.unlock();
	task->Init();
}

void XThread::AddTask(XTask* t)
{
	if (!t) return;
	t->base = this->base;
	tasks_mutex.lock();
	tasks.push_back(t);
	tasks_mutex.unlock();
}

//�̼߳���
void XThread::Activate()
{
#ifdef _WIN32
	int re = send(this->notify_send_fd, "c", 1, 0);
#else
	int re = write(this->notify_send_fd, "c", 1);
#endif
	if (re <= 0)
	{
		cerr << "XThread::Activate() failed!" << endl;
	}
}



void XThread::Start()
{
	Setup();
	//�����߳�
	thread th(&XThread::Main, this);

	//�Ͽ������߳���ϵ
	th.detach();
}

bool XThread::Setup()
{
	//windows����Ե�socket(ģ��ܵ�) linux�ùܵ�
#ifdef _WIN32
	//����һ��socketpair ���Ի���ͨ�� fds[0] ��; fds[1] д
	evutil_socket_t fds[2];
	if (evutil_socketpair(AF_INET, SOCK_STREAM, 0, fds) < 0)
	{
		cout << "evutil_socketpair failed!" << endl;
		return false;
	}
	evutil_make_socket_nonblocking(fds[0]);
	evutil_make_socket_nonblocking(fds[1]);
#else
	//�����Ĺܵ� ������send recv��ȡ read write
	int fds[2];
	if (pipe(fds))
	{
		cerr << "pipe failed!" << endl;
		return false;
	}
#endif

	//��ȡ�󶨵�event�¼��У�д��Ҫ����
	notify_send_fd = fds[1];

	//����libevent������(����)
	event_config* ev_conf = event_config_new();
	event_config_set_flag(ev_conf, EVENT_BASE_FLAG_NOLOCK);
	this->base = event_base_new_with_config(ev_conf);
	event_config_free(ev_conf);
	if (!base)
	{
		cerr << "event_base_new_with_config failed in thread!" << endl;
		return false;
	}

	//��ӹܵ������¼� ���ڼ����߳�ִ������

	event* ev = event_new(base, fds[0], EV_READ | EV_PERSIST, NotifyCallback, this);
	event_add(ev, 0);


	return true;
}

//�߳���ں���
void XThread::Main()
{
	cout << id << " XThread::Main() begin" << endl;
	event_base_dispatch(base);
	event_base_free(base);
	cout << id << " XThread::Main() end" << endl;
}

XThread::XThread()
{

}
XThread::~XThread()
{

}