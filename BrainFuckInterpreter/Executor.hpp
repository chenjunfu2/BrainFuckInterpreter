#pragma once

#include "CodeUnit.hpp"
#include "MemoryManager.hpp"
#include "MyAssert.hpp"

#include <vector>
#include <conio.h>

class Executor//执行器
{
private:
	size_t szCodeIndex = 0;

	CodeList listCode{};
	MemoryManager mMemory{};
public:
	Executor(void) = default;
	Executor(CodeList _listCode, size_t _szCodeIndex = 0, MemoryManager _mMemory = {}) :szCodeIndex(_szCodeIndex), listCode(std::move(_listCode)), mMemory(std::move(_mMemory))
	{
		MyAssert(mMemory, "致命错误：内存为NULL！");
	}
	~Executor(void) = default;

	Executor(const Executor &&) = delete;
	Executor(Executor &&_Move) :szCodeIndex(_Move.szCodeIndex), listCode(std::move(_Move.listCode)), mMemory(std::move(_Move.mMemory))
	{
		_Move.szCodeIndex = 0;

		MyAssert(mMemory, "致命错误：内存为NULL！");
	}

	Executor &operator=(const Executor &) = delete;
	Executor &operator=(Executor &&_Move)
	{
		szCodeIndex = _Move.szCodeIndex;
		_Move.szCodeIndex = 0;

		listCode = std::move(_Move.listCode);
		mMemory = std::move(_Move.mMemory);

		MyAssert(mMemory, "致命错误：内存为NULL！");
	}

	void SetMemory(MemoryManager _mMemory)
	{
		mMemory = std::move(_mMemory);

		MyAssert(mMemory, "致命错误：内存为NULL！");
	}

	void SetListCode(CodeList _listCode, size_t _szCodeIndex = 0)
	{
		szCodeIndex = _szCodeIndex;
		listCode = std::move(_listCode);
	}

	void ResetMemory(void)
	{
		mMemory.Reset();
	}

	const MemoryManager &GetMemory(void)
	{
		return mMemory;
	}

	const CodeList& GetListCode(void)
	{
		return listCode;
	}

	size_t GetCodeIndex(void)
	{
		return szCodeIndex;
	}

	bool Once(void)
	{
		MyAssert(szCodeIndex < listCode.size(), "致命错误：指令指针[%zu]超出索引范围[0~%zu]！", szCodeIndex, listCode.size() - 1);

		auto &curCode = listCode[szCodeIndex++];
		switch (curCode.enSymbol)
		{
		case CodeUnit::ProgEnd:
			return false;//直接结束
			break;
		case CodeUnit::NextMov:
			mMemory += curCode.szMovOffset;
			break;
		case CodeUnit::PrevMov:
			mMemory -= curCode.szMovOffset;
			break;
		case CodeUnit::AddCur:
			*mMemory += (uint8_t)curCode.szCalcValue;
			break;
		case CodeUnit::SubCur:
			*mMemory -= (uint8_t)curCode.szCalcValue;
			break;
		case CodeUnit::OptCur:
			_putch(*mMemory);
			break;
		case CodeUnit::IptCur:
			*mMemory = _getch();
			break;
		case CodeUnit::LoopBeg:
			if (*mMemory == 0)
			{
				szCodeIndex = curCode.szJmpIndex;//跳转
			}
			break;
		case CodeUnit::LoopEnd:
			szCodeIndex = curCode.szJmpIndex;//无条件跳转
			break;
		case CodeUnit::DbgInfo:
			printf("[%zu]:0x%02X\n", mMemory.GetIndex(), *mMemory);
			break;
		default:
			printf("致命错误：未知的指令！\n");
			return false;//注意直接返回失败
			break;
		}

		return true;
	}

	void Loop(void)
	{
		while (Once())
		{
			continue;
		}
	}

	//后面可以提供一些用于debug分析的函数，修改、显示内存，单步执行，断点，条件断点，内存断点，修改、显示指令等







};
