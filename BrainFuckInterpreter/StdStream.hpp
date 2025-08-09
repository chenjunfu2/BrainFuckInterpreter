#pragma once

#include <cstdio>

class StdStream
{
private:
	int iLast = '\0';//������һ��������unget
public:
	StdStream(void) = default;
	~StdStream(void) = default;

	operator bool(void) const//�������Ƿ����
	{
		return true;//��׼��Ӧ�����ǿ��ã���������޷���������
	}

	bool Eof(void) const
	{
		return feof(stdin) != 0;
	}

	bool Error(void) const
	{
		return ferror(stdin) != 0;
	}

	char GetChar(void)
	{
		iLast = getchar();
		return iLast;
	}

	bool UnGet(void)
	{
		return ungetc(iLast, stdin) != EOF;//ʧ�ܷ���EOF
	}
};