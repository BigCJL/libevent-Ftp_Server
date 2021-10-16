#include "XFtpLIST.h"
#include<iostream>
#include<event2/bufferevent.h>
#include<event.h>
#ifdef _WIN32
#include<io.h>  //用于遍历当前目录
#endif
using namespace std;

void XFtpLIST::Write(struct bufferevent* bev)
{
	//4 226 Transfer complete 发送完成
	ResCMD("226 Transfer complete\r\n");
	//5 关闭连接
	Close();
}
void XFtpLIST::Event(struct bufferevent* bev, short what)
{
	//如果对方网络断掉或者机器死机，有可能收不到BEV_EVENT_EOF的数据
	if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
	{
		cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR" << endl;
		Close();
	}
	else if (what & BEV_EVENT_CONNECTED)
	{
		cout << "XFtpLIST BEV_EVENT_CONNECTED" << endl;
	}

}

// 解析协议
void XFtpLIST::Parse(std::string type, std::string msg)
{
	string resmsg = "";
	if (type == "PWD")
	{
		//257 "/" is current directory
		resmsg = "257 \"";
		resmsg += cmdTask->curDir;
		resmsg += "\" is current dir.\r\n";
		ResCMD(resmsg);
	}
	else if (type == "LIST")
	{
		//1 连接数据通道 2 150发送成功 3 发送目录数据通道 4 发送完成226 5 关闭连接
		//命令通道回复消息，使用数据通道发送目录
		//-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jgp\r\n
		//1 连接数据通道
		ConnectPORT();
		//2 150
		ResCMD("150 Here comes thre directory listing.\r\n");
		//string listdata = "-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jgp\r\n";
		string listdata = GetListData(cmdTask->rootDir + cmdTask->curDir);
		//3 数据通道发送
		Send(listdata);
	}
	else if (type == "CWD")  //切换目录
	{
		//取出命令中的路径
		//CWD test\r\n
		int pos = msg.rfind(" ") + 1;
		//去掉结尾的\r\n
		string path = msg.substr(pos, msg.size() - pos - 2);
		if (path[0] == '/')  //绝对路径
		{
			cmdTask->curDir = path;
		}
		else
		{
			if (cmdTask->curDir[cmdTask->curDir.size() - 1] != '/')
				cmdTask->curDir += '/';
			cmdTask->curDir += path + "/";
		}


		// /test/
		ResCMD("250 Directory success changed.\r\n");

		//cmdTask->curDir +=
	}
	else if (type == "CDUP")  //回到上层路径
	{
		//  
		string path = cmdTask->curDir;
		//统一去掉结尾的 /
		if (path[path.size() - 1] == '/')
		{
			path = path.substr(0, path.size() - 1);
		}
		int pos = path.rfind("/");
		path = path.substr(0, pos);
		cmdTask->curDir = path;
		ResCMD("250 Directory success changed.\r\n");
	}
}

string XFtpLIST::GetListData(std::string path)
{
	//"-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jgp\r\n";
	string data = "";

#ifdef _WIN32
	//存储文件信息
	_finddata_t file;
	//目录上下文
	path += "/*.*";
	intptr_t dir = _findfirst(path.c_str(),&file);
	if (dir < 0)
	{
		return data;
	}
	do
	{
		string tmp = "";
		//是否是目录,去掉. ..
		if (file.attrib & _A_SUBDIR)
		{
			if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
			{
				continue;
			}
			tmp = "drwxrwxrwx 1 root group";
		}
		else
		{
			tmp = "-rwxrwxrwx 1 root group";
		}
		char buf[1024];
		sprintf(buf, "%u", file.size);
		tmp += buf;

		//日期时间
		strftime(buf, sizeof(buf) - 1, "%b %d %H:%M ", localtime(&file.time_write));
		tmp += buf;
		tmp += file.name;
		data += "\r\n";
		data += tmp;

	} while (_findnext(dir, &file) == 0);

#else
	string cmd = "ls -l ";
	cmd += path;
	cout << "Popen:" << cmd << endl;
	FILE* f = popen(cmd.c_str(), "r");
	if (!f)
		return data;
	char buffer[1024] = { 0 };
	for (;;)
	{
		int len = fread(buffer, 1, sizeof(buffer) - 1, f);
		if (len <= 0)break;
		buffer[len] = '\0';
		data += buffer;
	}
	pclose(f);
#endif
	return data;
}