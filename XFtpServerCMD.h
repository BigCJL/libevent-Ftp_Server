#pragma once
#include "XFtpTask.h"
#include <map>
class XFtpServerCMD : public XFtpTask
{
public:
    //��ʼ������
    virtual bool Init();
    virtual void Read(struct bufferevent* bev);
    virtual void Event(struct bufferevent* bev, short what);
    //ע����������(����Ҫ��Ҫ���ǣ��̻߳���Ҫ��Ҫ���ǣ�),ע��ʱ ��δ�ַ����߳�(Dispatch)
    void Reg(std::string s, XFtpTask* call);
private:
    std::map<std::string, XFtpTask*>calls;
};

