#pragma once

#include "CodeUnit.hpp"
#include "Executor.hpp"

#include "FileStream.hpp"
#include "StdStream.hpp"
#include "StrStream.hpp"
#include "Preprocessor.hpp"


class Interpreter//解释器
{
private:
	Executor exe{};
public:
	enum StreamType
	{
		Unknown,
		File,
		Str,
		Std,
	};

	Interpreter(const char *pStr, StreamType enStreamType, bool bIgnoreUnknownChar, bool bOptimization)
	{
		CodeList listCode{};

		switch (enStreamType)
		{
		case File:
			{
				FileStream sFile(pStr, "rb");
				if (!sFile)
				{
					printf("文件错误：打开失败\n");
					exit(-1);
				}

				if (!Preprocessor<FileStream>::PreprocessInStream(sFile, listCode, bIgnoreUnknownChar, bOptimization))
				{
					printf("预处理错误：翻译失败\n");
					exit(-1);
				}
			}
			break;
		case Str:
			{
				if (pStr == NULL)
				{
					printf("输入错误：输入为空\n");
					exit(-1);
				}

				StrStream csInput(pStr, strlen(pStr));//不用判断csInput有效性，此处默认一定成功
				if (!Preprocessor<StrStream>::PreprocessInStream(csInput, listCode, bIgnoreUnknownChar, bOptimization))
				{
					printf("预处理错误：翻译失败\n");
					exit(-1);
				}
			}
			break;
		case Std:
			{
				StdStream csInput{};
				if (!Preprocessor<StdStream>::PreprocessInStream(csInput, listCode, bIgnoreUnknownChar, bOptimization))
				{
					printf("预处理错误：翻译失败\n");
					exit(-1);
				}
			}
			break;
		case Unknown:
		default:
			printf("解析错误：未知类型\n");
			exit(-1);
			break;
		}

		if (listCode.empty())
		{
			printf("预处理错误：指令列表为空\n");
			exit(-1);
		}
		
		//不为空则初始化执行器
		exe.SetListCode(std::move(listCode));
	}

	void Run(void)
	{
		exe.Loop();
	}

	void Print(const char *pHead = "", const char *pTail = "", const char *pPer = "", const char *pSuf = " ")
	{
		PrintCodeList(exe.GetListCode(), pHead, pTail, pPer, pSuf);
	}
};