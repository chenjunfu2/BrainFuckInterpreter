#pragma once

#include "FileStream.hpp"
#include "CodeUnit.hpp"
#include "Preprocessor.hpp"
#include "Executor.hpp"


class Interpreter//解释器
{
	Executor exe{};
public:
	Interpreter(const char *pStr, bool bIsFile, bool bIgnoreUnknownChar)
	{
		CodeList listCode{};

		if (bIsFile)
		{
			FileStream sFile(pStr, "rb");
			if (!sFile)
			{
				printf("文件错误：打开失败\n");
				exit(-1);
			}

			if (!Preprocessor<FileStream>::PreprocessInFile(sFile, listCode, bIgnoreUnknownChar))
			{
				printf("预处理错误：翻译失败\n");
				exit(-1);
			}
		}
		else
		{



			CharStream csInput(pStr, strlen(pStr));//不用判断csInput有效性，此处默认一定成功
			CodeList listCode{};
			if (!Preprocessor<CharStream>::PreprocessInFile(csInput, listCode, bIgnoreUnknownChar))
			{
				printf("预处理错误：翻译失败\n");
				exit(-1);
			}
		}

		exe.SetListCode(std::move(listCode));//执行器初始化完成
	}

	void run(void)
	{
		exe.Loop();
	}
};