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
	//已经注册的是否覆盖？ ->不覆盖，提示错误
	if (calls.find(cmd) != calls.end())
	{
		cout << cmd << " is already register" << endl;
		return;
	}
	calls[cmd] = call;
}

//子线程XThread event事件分发
void XFtpServerCMD::Read(struct bufferevent* bev)
{
	char data[1024] = { 0 };
	for (;;)
	{
		int len = bufferevent_read(bev, data, sizeof(data) - 1);
		if (len <= 0)break;
		data[len] = '\0';
		cout <<"Recv CMD:"<<data << flush;
		//分发到处理对象
		//分析出类型 USER anonymous
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
			t->cmdTask = this;   //用来处理回复命令和目录
			t->Parse(type, data);
		}
		else
		{
			//如果没找到消息命令的类型,回复一个OK
			string msg = "200 OK\r\n";
			bufferevent_write(bev, msg.c_str(), msg.size());
		}

		/*
		//测试代码 要清理
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

	//如果对方网络断掉或者机器死机，有可能收不到BEV_EVENT_EOF的数据
	if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
	{
		cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR" << endl;
		bufferevent_free(bev);
		delete this;
	}
}





//初始化任务 运行在子线程中
bool XFtpServerCMD::Init()
{
	cout << "XFtpServerCMD::Init" << endl;
	//监听socket bufferevent
	//base socket
	bufferevent *bev = bufferevent_socket_new(base, sock, BEV_OPT_CLOSE_ON_FREE);  //base在子线程里的Setup初始化了
	this->bev = bev;
	this->SetCallback(bev);

	//添加超时 
	timeval rt = { 60,0 };
	bufferevent_set_timeouts(bev, &rt, 0);
	//发送欢迎消息 这里说明建立了一个连接
	string msg = "220 thist is libevent Ftp_Server, Welcome!\r\n";
	bufferevent_write(bev, msg.c_str(), msg.size());

	return true;
}