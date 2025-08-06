#pragma once

#include "CodeUnit.hpp"
#include "MemoryManager.hpp"
#include "MyAssert.hpp"

#include <vector>
#include <conio.h>

class Executor//ִ����
{
private:
	size_t szCodeIndex = 0;

	CodeList listCode{};
	MemoryManager mMemory{};
public:
	Executor(CodeList &&_listCode, size_t _szCodeIndex = 0, MemoryManager &&_managerMem = {}) :szCodeIndex(_szCodeIndex), listCode(std::move(_listCode)), mMemory(std::move(_managerMem))
	{}
	~Executor(void) = default;

	Executor(const Executor &&) = delete;
	Executor(Executor &&_Move) :szCodeIndex(_Move.szCodeIndex), listCode(std::move(_Move.listCode)), mMemory(std::move(_Move.mMemory))
	{
		_Move.szCodeIndex = 0;
	}

	Executor &operator=(const Executor &) = delete;
	Executor &operator=(Executor &&_Move)
	{
		szCodeIndex = _Move.szCodeIndex;
		_Move.szCodeIndex = 0;

		listCode = std::move(_Move.listCode);
		mMemory = std::move(_Move.mMemory);
	}

	bool Once(void)
	{
		MyAssert(szCodeIndex < listCode.size(), "��������ָ��ָ��[%zu]����������Χ[0~%zu]��", szCodeIndex, listCode.size() - 1);

		auto &curCode = listCode[szCodeIndex++];
		switch (curCode.enSymbol)
		{
		case CodeUnit::ProgEnd:
			{
				return false;
			}
			break;
		case CodeUnit::NextMov:
			{
				mMemory += curCode.szMovOffset;
			}
			break;
		case CodeUnit::PrevMov:
			{
				mMemory -= curCode.szMovOffset;
			}
			break;
		case CodeUnit::AddCur:
			{//ע�⣺���ڴ浥Ԫ��Ҳ����uint8_t�������㣬����ȷ����������ƣ����ڿ�Ԥ����Ϊ
				*mMemory += curCode.szCalcValue;
			}
			break;
		case CodeUnit::SubCur:
			{//ע�⣺���ڴ浥Ԫ��Ҳ����uint8_t�������㣬����ȷ����������ƣ����ڿ�Ԥ����Ϊ
				*mMemory -= curCode.szCalcValue;
			}
			break;
		case CodeUnit::OptCur:
			{
				putch(*mMemory);
			}
			break;
		case CodeUnit::IptCur:
			{
				*mMemory = _getch();
			}
			break;
		case CodeUnit::LoopBeg:
			{
				if (*mMemory == 0)
				{
					szCodeIndex = curCode.szJmpIndex;//��ת
				}
			}
			break;
		case CodeUnit::LoopEnd:
			{
				szCodeIndex = curCode.szJmpIndex;//��������ת
			}
			break;
		case CodeUnit::DbgInfo:
			{
				printf("\n[%zu]:0x%X\n", mMemory.GetIndex(), *mMemory);
			}
			break;
		default:
			MyAssert(false, "��������δ֪��ָ�");
			break;
		}
	}

	void Loop(void)
	{
		while (Once())
		{
			continue;
		}
	}

	//��������ṩһЩ����debug�����ĺ������޸ġ���ʾ�ڴ棬����ִ�У��ϵ㣬�����ϵ㣬�ڴ�ϵ㣬�޸ġ���ʾָ���







};
