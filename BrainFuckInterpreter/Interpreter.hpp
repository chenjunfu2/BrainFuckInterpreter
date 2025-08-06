#pragma once

#include "FileStream.hpp"
#include "CodeUnit.hpp"
#include "Preprocessor.hpp"
#include "Executor.hpp"


class Interpreter//解释器
{
	Executor exe{};
public:
	/*
	流程：
	先打开bf执行文件，
	用预处理器合并多个加减位移单元，
	用执行器执行
	*/
	Interpreter(const char *pFileName)
	{
		FileStream sFile(pFileName, "rb");
		if (!sFile)
		{
			printf("文件错误：打开失败！\n");
			exit(-1);
		}

		CodeList listCode{};
		if(!Preprocessor::PreprocessInFile(sFile, listCode))
		{
			printf("预处理错误：翻译失败！\n");
			exit(-1);
		}

		exe.SetListCode(std::move(listCode));//执行器初始化完成
	}

	void run(void)
	{
		exe.Loop();
	}
};