#pragma once

#include "MyAssert.hpp"

#include <cctype>
#include <map>
#include <functional>

#define ALPHABET_SIZE 26	//26英文字母

class CommandLineParser
{
private:
	using CallbackFunc = bool(char*);//回调函数
	using FuncType = std::function<CallbackFunc>;//回调函数类

	struct OptionInfo
	{
		bool bFind = false;//防止重复
		bool bRegister = false;//注册标志
		const char *pHelpMessage = NULL;//帮助信息
		FuncType funcCallback = {};//回调函数
	};

	OptionInfo listOptions[ALPHABET_SIZE] = {};
private:
	bool isCharLegality(char cTest)//仅允许'a'~'z'（无视大小写）
	{
		return isalpha(cTest) == 0;
	}

	int Char2Index(char cArg)
	{
		int iIndex = tolower(cArg) - (int)'a';
		MyAssert(iIndex >= 0 && iIndex < ALPHABET_SIZE, "致命错误：命令行参数索引溢出！");
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
		//		printf("解析失败：未知的字符\"%c\"", *pChar);
		//		return false;
		//	}
		//
		//	int iIndex = Char2Index(*pChar);
		//	auto &curOption = listOptions[iIndex];
		//	if (curOption.bRegister == false)
		//	{
		//		printf("解析失败：未知的选项\"%c\"", *pChar);
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