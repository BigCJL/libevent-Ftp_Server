#include "XFtpSTOR.h"
#include<iostream>
#include<event2/event.h>
#include<event2/bufferevent.h>
using namespace std;
//解析协议
void XFtpSTOR::Parse(std::string type, std::string msg)
{
	//文件名
	int pos = msg.rfind(" ") + 1;
	string filename = msg.substr(pos, msg.size() - pos - 2);
	string path = cmdTask->rootDir;
	path += cmdTask->curDir;
	path += filename;
	fp = fopen(path.c_str(), "wb");   //写入二进制
	if (fp)
	{
		//连接数据通道
		ConnectPORT();
		//发送开始接收文件的指令
		ResCMD("125 File OK\r\n");
		//触发读事件
		bufferevent_trigger(bev, EV_READ, 0);

	}
	else
	{
		ResCMD("450 file open failed!\r\n");
	}
}
void XFtpSTOR::Read(struct bufferevent* bev)
{
	if (!fp)return;
	for (;;)
	{
		int len = bufferevent_read(bev, buf, sizeof(buf));
		if (len <= 0)
			return;
		int size = fwrite(buf, 1, len, fp);
		cout << "<" << len << ":" << size << ">" << flush;
	}
}
void XFtpSTOR::Event(struct bufferevent* bev, short what)
{
	//如果对方网络断掉或者机器死机，有可能收不到BEV_EVENT_EOF的数据
	if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
	{
		cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR" << endl;
		Close();
		ResCMD("226 Transfer complete\r\n");
	}
	else if (what & BEV_EVENT_CONNECTED)
	{
		cout << "XFtpLIST BEV_EVENT_CONNECTED" << endl;
	}
}