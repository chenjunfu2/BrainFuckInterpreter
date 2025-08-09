#pragma once

#include <cstdio>

class StdStream
{
private:
	int iLast = '\0';//缓存上一个，用于unget
public:
	StdStream(void) = default;
	~StdStream(void) = default;

	operator bool(void) const//测试流是否可用
	{
		return true;//标准流应该总是可用，否则程序无法正常运行
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
		return ungetc(iLast, stdin) != EOF;//失败返回EOF
	}
};