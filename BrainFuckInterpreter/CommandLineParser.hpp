#pragma once

#include "MyAssert.hpp"

#include <cctype>
#include <map>
#include <functional>

#define ALPHABET_SIZE 26	//26Ӣ����ĸ

class CommandLineParser
{
private:
	using CallbackFunc = bool(char*);//�ص�����
	using FuncType = std::function<CallbackFunc>;//�ص�������

	struct OptionInfo
	{
		bool bFind = false;//��ֹ�ظ�
		bool bRegister = false;//ע���־
		const char *pHelpMessage = NULL;//������Ϣ
		FuncType funcCallback = {};//�ص�����
	};

	OptionInfo listOptions[ALPHABET_SIZE] = {};
private:
	bool isCharLegality(char cTest)//������'a'~'z'�����Ӵ�Сд��
	{
		return isalpha(cTest) == 0;
	}

	int Char2Index(char cArg)
	{
		int iIndex = tolower(cArg) - (int)'a';
		MyAssert(iIndex >= 0 && iIndex < ALPHABET_SIZE, "�������������в������������");
		return iIndex;
	}

public:
	CommandLineParser(void) = default;
	~CommandLineParser(void) = default;


	bool Parse(const char *pcArg)
	{
		//for (const char *pChar = pcArg; *pChar != '\0'; ++pChar)
		//{
		//	if (!isCharLegality(*pChar))
		//	{
		//		printf("����ʧ�ܣ�δ֪���ַ�\"%c\"", *pChar);
		//		return false;
		//	}
		//
		//	int iIndex = Char2Index(*pChar);
		//	auto &curOption = listOptions[iIndex];
		//	if (curOption.bRegister == false)
		//	{
		//		printf("����ʧ�ܣ�δ֪��ѡ��\"%c\"", *pChar);
		//	}
		//
		//	if (!curOption.funcCallback())
		//	{
		//		return false;
		//	}
		//}
		//
		//return true;
	}


	void FindClear(void)
	{
		for (auto &it : listOptions)
		{
			it.bFind = false;
		}
	}

	bool Register(char cArg, FuncType funcCallback, const char *pHelpMessage)
	{
		if (!isCharLegality(cArg))
		{
			return false;
		}

		int iIndex = Char2Index(cArg);
		auto &curOption = listOptions[iIndex];
		curOption.bFind = false;
		curOption.bRegister = true;
		curOption.pHelpMessage = pHelpMessage;
		curOption.funcCallback = std::move(funcCallback);

		return true;
	}

	bool Unregister(char cArg)
	{
		if (!isCharLegality(cArg))
		{
			return false;
		}

		int iIndex = Char2Index(cArg);
		auto &curOption = listOptions[iIndex];
		curOption.bFind = false;
		curOption.bRegister = false;
		curOption.pHelpMessage = NULL;
		curOption.funcCallback = {};

		return true;
	}


};

#undef ALPHABET_SIZE