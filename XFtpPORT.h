#pragma once
#include "XFtpTask.h"
class XFtpPORT :
    public XFtpTask
{
public:
    //����Э��
    void Parse(std::string type, std::string msg);
};

