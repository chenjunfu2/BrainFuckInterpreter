#include "Interpreter.hpp"

#include <stdio.h>

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("仅接受一个brainfuck文件名参数！");
		return 0;
	}

	try
	{
		Interpreter itp(argv[1]);
		itp.run();
	}
	catch (const std::bad_alloc &)
	{
		printf("运行错误：内存不足！\n");
		exit(-1);
	}

	return 0;
}