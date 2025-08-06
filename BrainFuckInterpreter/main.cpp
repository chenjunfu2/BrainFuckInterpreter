#include "Interpreter.hpp"

#include <stdio.h>
#include <new>

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("命令行错误：仅接受一个文件路径参数！\n");
		exit(-1);
	}

	try
	{
		Interpreter itp(argv[1]);
		itp.run();
	}
	catch (const std::bad_alloc &e)
	{
		printf("运行错误：内存不足！[%s]\n", e.what());//标准库异常
		exit(-1);
	}

	return 0;
}