#pragma once

#include "CodeUnit.hpp"
#include "Executor.hpp"

#include "FileStream.hpp"
#include "StdStream.hpp"
#include "StrStream.hpp"
#include "Preprocessor.hpp"


class Interpreter//������
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
					printf("�ļ����󣺴�ʧ��\n");
					exit(-1);
				}

				if (!Preprocessor<FileStream>::PreprocessInStream(sFile, listCode, bIgnoreUnknownChar, bOptimization))
				{
					printf("Ԥ������󣺷���ʧ��\n");
					exit(-1);
				}
			}
			break;
		case Str:
			{
				if (pStr == NULL)
				{
					printf("�����������Ϊ��\n");
					exit(-1);
				}

				StrStream csInput(pStr, strlen(pStr));//�����ж�csInput��Ч�ԣ��˴�Ĭ��һ���ɹ�
				if (!Preprocessor<StrStream>::PreprocessInStream(csInput, listCode, bIgnoreUnknownChar, bOptimization))
				{
					printf("Ԥ������󣺷���ʧ��\n");
					exit(-1);
				}
			}
			break;
		case Std:
			{
				StdStream csInput{};
				if (!Preprocessor<StdStream>::PreprocessInStream(csInput, listCode, bIgnoreUnknownChar, bOptimization))
				{
					printf("Ԥ������󣺷���ʧ��\n");
					exit(-1);
				}
			}
			break;
		case Unknown:
		default:
			printf("��������δ֪����\n");
			exit(-1);
			break;
		}

		if (listCode.empty())
		{
			printf("Ԥ�������ָ���б�Ϊ��\n");
			exit(-1);
		}
		
		//��Ϊ�����ʼ��ִ����
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