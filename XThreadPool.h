#pragma once
#include<vector>
#include "XTask.h"

class XThread;
class XThreadPool
{
public:
	//����ģʽ
	static XThreadPool* Get()
	{
		static XThreadPool p;
		return &p;
	}
	//��ʼ�������߳� �������߳�
	void Init(int threadCount);

	//�ַ��߳�
	void Dispatch(XTask* task);

private:
	//�߳�����
	int threadCount = 0;
	int lastThread = -1;
	//�̳߳��߳�
	std::vector<XThread*> threads;
	XThreadPool() {};   //С���� ���캯��˽�� ������ʵ����
};

