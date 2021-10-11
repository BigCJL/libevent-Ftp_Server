#pragma once
#include "XFtpTask.h"
class XFtpUSER : public XFtpTask
{
public:
    virtual void Parse(std::string type, std::string msg);
};

