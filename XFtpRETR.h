#pragma once
#include "XFtpTask.h"
class XFtpRETR :
    public XFtpTask
{
public:
    virtual void Parse(std::string type, std::string msg);
    virtual void Write(struct bufferevent* bev);
    virtual void Event(struct bufferevent* bev, short what);

private:
    //FILE* fp = 0;
    char buf[1024];
};

