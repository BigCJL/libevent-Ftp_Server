#pragma once
class XTask
{
public:
	struct event_base* base = 0;
	int sock;
	int thread_id = 0;
	//��ʼ������
	virtual bool Init() = 0;
};

