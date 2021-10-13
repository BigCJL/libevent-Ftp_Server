#include "XFtpTask.h"
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <iostream>
using namespace std;
//回复cmd消息
void XFtpTask::ResCMD(string msg)
{
	if (!cmdTask || !cmdTask->bev) return;
	cout << "ResCMD:" << msg << endl;
	if (msg[msg.size() - 1] != '\n')
		msg += "\r\n";
	bufferevent_write(cmdTask->bev, msg.c_str(), msg.size());
}

void XFtpTask::SetCallback(struct bufferevent* bev)
{
	bufferevent_setcb(bev, ReadCB, 0, EventCB, this);
	bufferevent_enable(bev, EV_READ | EV_WRITE);
}
void XFtpTask::ReadCB(bufferevent* bev, void* arg)
{
	//XFtpTask* t = (XFtpTask*)arg;   c风格强制转换
	XFtpTask* t = static_cast<XFtpTask*>(arg);//cpp风格强制转换
	t->Read(bev);
}
void XFtpTask::WriteCB(bufferevent* bev, void* arg)
{
	XFtpTask* t = static_cast<XFtpTask*>(arg);
	t->Write(bev);
}
void XFtpTask::EventCB(struct bufferevent* bev, short what, void* arg)
{
	XFtpTask* t = static_cast<XFtpTask*>(arg);
	t->Event(bev, what);
}
