#include "XFtpTask.h"
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <iostream>
#include <cstring>
using namespace std;

void XFtpTask::Send(std::string data)
{
	Send(data.c_str(), data.size());
}
void XFtpTask::Send(const char* data, int datasize)
{
	if (!bev) return;
	bufferevent_write(bev, data, datasize);
}
void XFtpTask::Close()
{
	if (bev)  //��ֹ�����ͷ� �ڴ�
	{
		bufferevent_free(bev);
		bev = 0;
	}
	if (fp)
	{
		fclose(fp);
		fp = 0;
	}
}


//��������ͨ��
void XFtpTask::ConnectPORT()
{
	if (ip.empty() || port <= 0 || !base)
	{
		cout << "ConnectPORT failed ip or port is null" << endl;
		return;
	}
	bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
	sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	evutil_inet_pton(AF_INET, ip.c_str(), &sin.sin_addr.s_addr);
	//���ûص���Ȩ����
	SetCallback(bev);
	//��ӳ�ʱ 
	timeval rt = { 60,0 };
	bufferevent_set_timeouts(bev, &rt, 0);
	bufferevent_socket_connect(bev, (sockaddr*)&sin, sizeof(sin));
}


//�ظ�cmd��Ϣ
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
	bufferevent_setcb(bev, ReadCB, WriteCB, EventCB, this);
	bufferevent_enable(bev, EV_READ | EV_WRITE);
}
void XFtpTask::ReadCB(bufferevent* bev, void* arg)
{
	//XFtpTask* t = (XFtpTask*)arg;   c���ǿ��ת��
	XFtpTask* t = static_cast<XFtpTask*>(arg);//cpp���ǿ��ת��
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
