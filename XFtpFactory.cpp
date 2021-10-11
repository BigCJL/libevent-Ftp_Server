#include "XFtpFactory.h"
#include "XFtpServerCMD.h"
#include "XFtpUSER.h"
XTask* XFtpFactory::CreateTask()
{
	XFtpServerCMD* x = new XFtpServerCMD();
	
	//注册ftp消息处理对象
	x->Reg("USER", new XFtpUSER());

	return x;
}

XFtpFactory::XFtpFactory()
{

}