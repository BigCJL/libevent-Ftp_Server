#include "XFtpLIST.h"
#include<iostream>
using namespace std;
// ½âÎöÐ­Òé
void XFtpLIST::Parse(std::string type, std::string msg)
{
	string resmsg = "";
	if (type == "PWD")
	{
		//257 "/" is current directory
		resmsg = "257 \"";
		resmsg += cmdTask->curDir;
		resmsg += "\" is current dir.\r\n";
		ResCMD(resmsg);
	}
}