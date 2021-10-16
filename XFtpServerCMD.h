#pragma once
#include "XFtpTask.h"
#include <map>
class XFtpServerCMD : public XFtpTask
{
public:
    //初始化任务
    virtual bool Init();
    virtual void Read(struct bufferevent* bev);
    virtual void Event(struct bufferevent* bev, short what);
    //注册命令处理对象(开销要不要考虑？线程互斥要不要考虑？),注册时 还未分发到线程(Dispatch)
    void Reg(std::string s, XFtpTask* call);
    ~XFtpServerCMD();

private:
    //注册的处理对象
    std::map<std::string, XFtpTask*>calls;
    //用来做空间清理
    std::map<XFtpTask*, int>calls_del;
};

