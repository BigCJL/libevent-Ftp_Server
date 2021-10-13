#include "XFtpServerCMD.h"
#include <event2/bufferevent.h>
#include <event2/event.h>
#include<iostream>
#include<string.h>
using namespace std;

void XFtpServerCMD::Reg(std::string cmd, XFtpTask* call)
{
	if (!call)
	{
		cout << "XFtpServerCMD::Reg call is null" << endl;
		return;
	}
	if (cmd.empty())
	{
		cout << "XFtpServerCMD::Reg cmd is null" << endl;
		return;
	}
	//�Ѿ�ע����Ƿ񸲸ǣ� ->�����ǣ���ʾ����
	if (calls.find(cmd) != calls.end())
	{
		cout << cmd << " is already register" << endl;
		return;
	}
	calls[cmd] = call;
}

//���߳�XThread event�¼��ַ�
void XFtpServerCMD::Read(struct bufferevent* bev)
{
	char data[1024] = { 0 };
	for (;;)
	{
		int len = bufferevent_read(bev, data, sizeof(data) - 1);
		if (len <= 0)break;
		data[len] = '\0';
		cout <<"Recv CMD:"<<data << flush;
		//�ַ����������
		//���������� USER anonymous
		string type = "";
		for (int i = 0; i < len; i++)
		{
			if (data[i] == ' ' || data[i] == '\r')
				break;
			type += data[i];
		}
		cout << "type is [" << type << "]" << endl;
		if (calls.find(type) != calls.end())
		{
			XFtpTask* t = calls[type];
			t->cmdTask = this;   //��������ظ������Ŀ¼
			t->Parse(type, data);
		}
		else
		{
			//���û�ҵ���Ϣ���������,�ظ�һ��OK
			string msg = "200 OK\r\n";
			bufferevent_write(bev, msg.c_str(), msg.size());
		}

		/*
		//���Դ��� Ҫ����
		if (strstr(data, "quit"))
		{
			bufferevent_free(bev);
			delete this;
			break;
		}
		*/
	}
}
void XFtpServerCMD::Event(struct bufferevent* bev, short what)
{

	//����Է�����ϵ����߻����������п����ղ���BEV_EVENT_EOF������
	if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
	{
		cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR" << endl;
		bufferevent_free(bev);
		delete this;
	}
}





//��ʼ������ ���������߳���
bool XFtpServerCMD::Init()
{
	cout << "XFtpServerCMD::Init" << endl;
	//����socket bufferevent
	//base socket
	bufferevent *bev = bufferevent_socket_new(base, sock, BEV_OPT_CLOSE_ON_FREE);  //base�����߳����Setup��ʼ����
	this->bev = bev;
	this->SetCallback(bev);

	//��ӳ�ʱ 
	timeval rt = { 60,0 };
	bufferevent_set_timeouts(bev, &rt, 0);
	//���ͻ�ӭ��Ϣ ����˵��������һ������
	string msg = "220 thist is libevent Ftp_Server, Welcome!\r\n";
	bufferevent_write(bev, msg.c_str(), msg.size());

	return true;
}