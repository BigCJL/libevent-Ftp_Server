#pragma once
#include "XTask.h"
#include <string>
class XFtpTask : public XTask
{
public:
	struct bufferevent* cmdbev = 0;
	//解析协议
	virtual void Parse(std::string type, std::string msg){}
	//回复cmd消息
	void ResCMD(std::string msg);
	virtual void Read(struct bufferevent *bev){}
	virtual void Write(struct bufferevent* bev) {}
	virtual void Event(struct bufferevent* bev, short what) {}
	void SetCallback(struct bufferevent* bev);
	bool Init() { return true; }
protected:
	static void ReadCB(bufferevent* bev, void* arg);
	static void WriteCB(bufferevent* bev, void* arg);
	static void EventCB(struct bufferevent* bev, short what, void* arg);
	//命令bev

};

