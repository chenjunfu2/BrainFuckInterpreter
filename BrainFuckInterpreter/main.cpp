#include "Interpreter.hpp"

#include <stdio.h>
#include <new>

#define PROJECT "BrainFuckInterpreter"
#define VERSION "1.0.6"

#define COPYRIGHT "Copyright(C)"
#define YEAR "2025"
#define AUTHOR "chenjunfu2"


void PrintHead(void)
{
	printf("%s [V%s]\n%s %s %s\n\n", PROJECT, VERSION, COPYRIGHT, YEAR, AUTHOR);
}


void PrintHelp(void)
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
		"   SELF [-g/-o/-s/-r ([i/o] [n]/[l]/[f {FileName}])] ([-f {FileName}]/[-a {Input}]/[-i])\n"\
		"      -g        忽略未知字符，不报错\n"\
		"      -o        对代码进行优化\n"\
		"      -s        输出优化后的代码\n"\
		"      -r        重定向流（由后续字符指定）\n"\
		"            i      重定向流Input\n"\
		"            o      重定向流Output\n"\
		"            n      重定向流为无缓冲流\n"\
		"            l      重定向流为行缓冲流\n"\
		"            f      重定向流为文件流\n"\
		"            FileName   需要重定向的文件流的文件名\n"\
		"\n"\
		"      -f        从文件中打开并运行BF代码\n"\
		"      FileName  需要运行的BF代码文件名\n"\
		"      -a        从参数中输入并运行BF代码\n"\
		"      Input     需要运行的BF代码（参数中输入记得加双引号）\n"\
		"      -i        从标准流中输入并运行BF代码\n"\
		"\n"\
	);
}

bool IsOption(const char *pInput)
{
	return pInput[0] == '-' || pInput[0] == '/';
}


int main(int argc, const char *argv[])
{
	PrintHead();//输出固定程序信息头

	if (argc < 2)
	{
		PrintHelp();
		exit(0);
	}

	//判断命令行选项
	bool bGetG = false;
	bool bGetO = false;
	bool bGetS = false;
	bool bGetFAI = false;

	bool bIgnoreUnknownChar = false;
	bool bOptimization = false;
	bool bShowCode = false;

	Interpreter::StreamType sType = Interpreter::StreamType::Unknown;

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
		case 'o':
			{
				if (bGetO != false)
				{
					goto Error;
				}

				bOptimization = true;
			}
			break;
		case 's':
			{
				if (bGetS != false)
				{
					goto Error;
				}

				bShowCode = true;
			}
			break;
		case 'f'://输入为文件
			{
				if (bGetFAI != false)
				{
					goto Error;
				}
				bGetFAI = true;

				sType = Interpreter::StreamType::File;

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
		case 'a'://输入为参数args
			{
				if (bGetFAI != false)
				{
					goto Error;
				}
				bGetFAI = true;

				sType = Interpreter::StreamType::Str;

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
		case 'i'://输入为标准io
			{
				if (bGetFAI != false)
				{
					goto Error;
				}
				bGetFAI = true;

				sType = Interpreter::StreamType::Std;//type为std则直接忽略pInput

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
			PrintHelp();
			exit(0);
			break;
		}
	}

	try
	{
		Interpreter itp(pInput, sType, bIgnoreUnknownChar, bOptimization);
		if (bShowCode)
		{
			itp.Print("代码显示: ", "\n\n");
		}

		itp.Run();
	}
	catch (const std::bad_alloc &e)
	{
		printf("运行错误：内存不足[%s]\n", e.what());//标准库异常
		exit(-1);
	}

	putchar('\n');
	return 0;
}

/*
TODO:
调试模式，@代码内显示断点，允许多种执行方式，查看内存
编译为中间码并保存到文件，从中间码文件中执行
支持从文件中重定向输入给bf程序，bf程序输出重定向到文件中
输出为cpp代码
*/