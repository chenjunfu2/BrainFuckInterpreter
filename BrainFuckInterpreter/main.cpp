#include "Interpreter.hpp"

#include <stdio.h>
#include <new>

#define PROJECT "BrainFuckInterpreter"
#define VERSION "1.0.2"

#define COPYRIGHT "CopyRight(C)"
#define YEAR "2025"
#define AUTHOR "chenjunfu2"


void Head(void)
{
	printf("%s [V%s]\n%s %s %s\n\n", PROJECT, VERSION, COPYRIGHT, YEAR, AUTHOR);
}


void Help(void)
{
	printf
	(
		"命令行说明：\n"\
		"   - SELF指代程序自身名字\n"\
		"   - 方括号[]内为可选项\n"\
		"   - 圆括号()内为必选项\n"\
		"   - 花括号{}内为参数名\n"\
		"   - 多个选项用/隔开\n"\
		"   - 以-开头的选项大小写敏感\n"\
		"   - 选项开头符号-与/等价\n"\
		"\n"\
		"命令行选项：\n"\
		"   SELF [-g] ([-f {FileName}]/[-i {Input}])\n"\
		"      -g        忽略未知字符，不报错\n"\
		"      -f        从文件中打开并运行BF代码\n"\
		"      -i        从标准流中输入并运行BF代码\n"\
		"      FileName  需要运行的BF代码文件名\n"\
		"      Input     需要运行的BF代码(直接输入)\n"\
	);
}

bool IsOption(const char *pInput)
{
	return pInput[0] == '-' || pInput[0] == '/';
}


int main(int argc, const char *argv[])
{
	Head();

	if (argc < 3 || argc > 4)
	{
		printf("命令行参数数量错误！\n");
		Help();
		exit(0);
	}

	//判断命令行选项
	bool bGetG = false;
	bool bGetFI = false;

	bool bIgnoreUnknownChar = false;
	bool bIsFile = false;

	const char *pInput = NULL;

	//这里一定要用<而非!=，因为内部可能过分消耗导致提前让i大等于argc，
	//这种情况下for末尾还会进行一次递增导致无法判断到!=导致错误
	for (int i = 1; i < argc; ++i)
	{
		if (!IsOption(argv[i]))
		{
			goto Error;
		}

		switch (argv[i][1])
		{
		case 'g'://忽略未知字符
			{
				if (bGetG != false)
				{
					goto Error;
				}

				bIgnoreUnknownChar = true;
			}
			break;
		case 'f'://输入为文件
			{//bIsFile本身为false如果走下面的i则仍然为false，如果走这里，设置为true并fallthrough到下一个case内
				bIsFile = true;
			}
			[[fallthrough]];//标记非break，消警告
		case 'i'://输入为参数
			{
				if (bGetFI != false)
				{
					goto Error;
				}
				bGetFI = true;

				if (++i < argc)//确认下一个命令行参数存在
				{
					pInput = argv[i];//作为入参
				}
				else
				{
					--i;//无参数，回退
				}

				//要求输入必须是最后一个，其他选项必须在前面，如果现在后面还有内容，则报错
				if (++i < argc)//如果不小于argc，那就直接走for判断退出
				{
					goto Error;
				}
			}
			break;
		default:
		Error:
			printf("命令行参数未知或重复！\n");
			Help();
			exit(0);
			break;
		}
	}

	try
	{
		Interpreter itp(pInput, bIsFile, bIgnoreUnknownChar);
		itp.run();
	}
	catch (const std::bad_alloc &e)
	{
		printf("运行错误：内存不足[%s]\n", e.what());//标准库异常
		exit(-1);
	}

	return 0;
}