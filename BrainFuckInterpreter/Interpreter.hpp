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
	Interpreter(const char *pFileName)
	{
		FileStream sFile(pFileName, "rb");
		if (!sFile)
		{
			printf("�ļ����󣺴�ʧ�ܣ�\n");
			exit(-1);
		}

		CodeList listCode{};
		if(!Preprocessor::PreprocessInFile(sFile, listCode))
		{
			printf("Ԥ������󣺷���ʧ�ܣ�\n");
			exit(-1);
		}

		exe.SetListCode(std::move(listCode));//ִ������ʼ�����
	}

	void run(void)
	{
		exe.Loop();
	}
};