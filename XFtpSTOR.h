#pragma once
#include "XFtpTask.h"

//�ϴ��ļ�
class XFtpSTOR :
    public XFtpTask
{
public:
    //����Э��
    virtual void Parse(std::string type, std::string msg);
    virtual void Read(struct bufferevent* bev);
    virtual void Event(struct bufferevent* bev, short what);
private:
    //FILE* fp = 0;
    char buf[1024];
};

