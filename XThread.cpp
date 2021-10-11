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

//激活线程任务的回调函数
static void NotifyCallback(evutil_socket_t fd, short which, void* arg)
{
	XThread* t = (XThread*)arg;
	t->Notify(fd, which);

}

void XThread::Notify(evutil_socket_t fd, short which)
{
	//水平触发！只要没有接收完全 会再次进来
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
	//获取任务 并初始化任务
	tasks_mutex.lock();
	if (tasks.empty())
	{
		tasks_mutex.unlock();
		return;
	}
	task = tasks.front();  //先进先出
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

//线程激活
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
	//启动线程
	thread th(&XThread::Main, this);

	//断开与主线程联系
	th.detach();
}

bool XThread::Setup()
{
	//windows用配对的socket(模拟管道) linux用管道
#ifdef _WIN32
	//创建一个socketpair 可以互相通信 fds[0] 读; fds[1] 写
	evutil_socket_t fds[2];
	if (evutil_socketpair(AF_INET, SOCK_STREAM, 0, fds) < 0)
	{
		cout << "evutil_socketpair failed!" << endl;
		return false;
	}
	evutil_make_socket_nonblocking(fds[0]);
	evutil_make_socket_nonblocking(fds[1]);
#else
	//创建的管道 不能用send recv读取 read write
	int fds[2];
	if (pipe(fds))
	{
		cerr << "pipe failed!" << endl;
		return false;
	}
#endif

	//读取绑定到event事件中，写入要保存
	notify_send_fd = fds[1];

	//创建libevent上下文(无锁)
	event_config* ev_conf = event_config_new();
	event_config_set_flag(ev_conf, EVENT_BASE_FLAG_NOLOCK);
	this->base = event_base_new_with_config(ev_conf);
	event_config_free(ev_conf);
	if (!base)
	{
		cerr << "event_base_new_with_config failed in thread!" << endl;
		return false;
	}

	//添加管道监听事件 用于激活线程执行任务

	event* ev = event_new(base, fds[0], EV_READ | EV_PERSIST, NotifyCallback, this);
	event_add(ev, 0);


	return true;
}

//线程入口函数
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