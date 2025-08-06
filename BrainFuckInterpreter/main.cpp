#include <stdio.h>
#include "Interpreter.hpp"

int main(int argc, char *argv[], char *envp[])
{
	//if (argc != 2)
	//{
	//	printf("仅接受一个brainfuck文件名参数！");
	//	return 0;
	//}

	//Interpreter itp(argv[1]);
	Interpreter itp("test.bf");
	itp.run();

	return 0;
}