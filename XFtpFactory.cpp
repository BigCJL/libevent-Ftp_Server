#include "XFtpFactory.h"
#include "XFtpServerCMD.h"
#include "XFtpUSER.h"
XTask* XFtpFactory::CreateTask()
{
	XFtpServerCMD* x = new XFtpServerCMD();
	
	//ע��ftp��Ϣ�������
	x->Reg("USER", new XFtpUSER());

	return x;
}

XFtpFactory::XFtpFactory()
{

}