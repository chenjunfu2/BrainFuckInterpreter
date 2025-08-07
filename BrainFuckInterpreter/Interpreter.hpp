#pragma once

#include "FileStream.hpp"
#include "CodeUnit.hpp"
#include "Preprocessor.hpp"
#include "Executor.hpp"


class Interpreter//������
{
	Executor exe{};
public:
	/*
	���̣�
	�ȴ�bfִ���ļ���
	��Ԥ�������ϲ�����Ӽ�λ�Ƶ�Ԫ��
	��ִ����ִ��
	*/
	Interpreter(const char *pFileName, bool bIgnoreUnknownChar)
	{
		FileStream sFile(pFileName, "rb");
		if (!sFile)
		{
			printf("�ļ����󣺴�ʧ��\n");
			exit(-1);
		}

		CodeList listCode{};
		if(!Preprocessor::PreprocessInFile(sFile, listCode, bIgnoreUnknownChar))
		{
			printf("Ԥ������󣺷���ʧ��\n");
			exit(-1);
		}

		exe.SetListCode(std::move(listCode));//ִ������ʼ�����
	}

	void run(void)
	{
		exe.Loop();
	}
};