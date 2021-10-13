#pragma once
#include "XFtpTask.h"
class XFtpLIST :
    public XFtpTask
{
    virtual void Parse(std::string type, std::string msg);
};

