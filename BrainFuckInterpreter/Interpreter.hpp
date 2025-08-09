#pragma once

#include "FileStream.hpp"
#include "CodeUnit.hpp"
#include "Preprocessor.hpp"
#include "Executor.hpp"


class Interpreter//������
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
				printf("�ļ����󣺴�ʧ��\n");
				exit(-1);
			}

			if (!Preprocessor<FileStream>::PreprocessInFile(sFile, listCode, bIgnoreUnknownChar))
			{
				printf("Ԥ������󣺷���ʧ��\n");
				exit(-1);
			}
		}
		else
		{



			CharStream csInput(pStr, strlen(pStr));//�����ж�csInput��Ч�ԣ��˴�Ĭ��һ���ɹ�
			CodeList listCode{};
			if (!Preprocessor<CharStream>::PreprocessInFile(csInput, listCode, bIgnoreUnknownChar))
			{
				printf("Ԥ������󣺷���ʧ��\n");
				exit(-1);
			}
		}

		exe.SetListCode(std::move(listCode));//ִ������ʼ�����
	}

	void run(void)
	{
		exe.Loop();
	}
};