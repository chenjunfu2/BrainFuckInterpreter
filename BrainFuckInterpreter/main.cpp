#include <stdio.h>
#include "Interpreter.hpp"

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("仅接受一个brainfuck文件名参数！");
		return 0;
	}

	Interpreter itp(argv[1]);
	itp.run();

	return 0;
}