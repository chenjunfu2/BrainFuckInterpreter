#include "Interpreter.hpp"

#include <stdio.h>
#include <new>

int main(int argc, const char *argv[])
{
	if (argc < 2 || argc > 3)
	{
		printf
		(
			"命令行说明：\n"\
			"   1.SELF指代程序自身名字\n"\
			"   2.方括号[]内为可选项，多个选项用/隔开\n"\
			"   3.圆括号()内为必选项\n"\
			"   4.符号\\与/与-等价\n"\
			"   5.选项大小写敏感\n"\
			"\n"\
			"命令行选项：\n"\
			"   SELF [-iuc] (FileName)\n"\
			"      -iuc      忽略未知字符，不报错\n"\
			"      FileName  需要运行的BF代码文件名\n"\
		);
		exit(0);
	}

	//判断命令行选项
	bool bIgnoreUnknownChar = false;
	if (argv[1][0] == '\\' || argv[1][0] == '/' || argv[1][0] == '-')
	{
		if (strcmp(argv[1], "iuc") != 0)
		{
			printf("命令行参数错误：未知的选项\"%s\"\n", argv[1]);
			exit(-1);
		}
		bIgnoreUnknownChar = true;
	}

	//作为文件名
	const char *pFileName = argv[argc - 1];

	try
	{
		Interpreter itp(pFileName, bIgnoreUnknownChar);
		itp.run();
	}
	catch (const std::bad_alloc &e)
	{
		printf("运行错误：内存不足[%s]\n", e.what());//标准库异常
		exit(-1);
	}

	return 0;
}