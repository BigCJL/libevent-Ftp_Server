#pragma once
#include "XFtpTask.h"
class XFtpPORT :
    public XFtpTask
{
public:
    //½âÎöĞ­Òé
    void Parse(std::string type, std::string msg);
};

