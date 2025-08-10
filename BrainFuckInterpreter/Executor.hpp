#pragma once

#include "CodeUnit.hpp"
#include "MemoryManager.hpp"
#include "IO.hpp"
#include "MyAssert.hpp"

#include <vector>

class Executor//ִ����
{
private:
	size_t szCodeIndex = 0;

	IO io{};
	CodeList listCode{};
	MemoryManager mMemory{};
public:
	Executor(void) = default;
	Executor(CodeList _listCode, size_t _szCodeIndex = 0, MemoryManager _mMemory = {}, IO _io = {}) :szCodeIndex(_szCodeIndex), io(_io), listCode(std::move(_listCode)), mMemory(std::move(_mMemory))
	{}
	~Executor(void) = default;

	Executor(const Executor &&) = delete;
	Executor(Executor &&_Move) :szCodeIndex(std::move(_Move.szCodeIndex)), io(std::move(_Move.io)), listCode(std::move(_Move.listCode)), mMemory(std::move(_Move.mMemory))
	{
		_Move.szCodeIndex = 0;//�����Ա��Ҫ�ֶ���������
	}

	Executor &operator=(const Executor &) = delete;
	Executor &operator=(Executor &&_Move)
	{
		szCodeIndex = std::move(_Move.szCodeIndex);
		_Move.szCodeIndex = 0;//�����Ա��Ҫ�ֶ���������

		//���Ա�������ƶ���ֵ����
		io = std::move(_Move.io);
		listCode = std::move(_Move.listCode);
		mMemory = std::move(_Move.mMemory);
	}

	void SetMemory(MemoryManager _mMemory)
	{
		mMemory = std::move(_mMemory);
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
		if (szCodeIndex >= listCode.size())
		{
			printf("��������ָ��ָ��[%zu]����������Χ[0~%zu)\n", szCodeIndex, listCode.size());
			exit(-1);
		}

		auto &curCode = listCode[szCodeIndex++];
		switch (curCode.enSymbol)
		{
		case CodeUnit::ProgEnd://������־
			return false;//ֱ�ӽ���
			break;
		case CodeUnit::NextMov:
			mMemory += curCode.szMovOffset;
			break;
		case CodeUnit::PrevMov:
			mMemory -= curCode.szMovOffset;
			break;
		case CodeUnit::AddCur:
			*mMemory += curCode.u8CalcValue;
			break;
		case CodeUnit::SubCur:
			*mMemory -= curCode.u8CalcValue;
			break;
		case CodeUnit::OptCur:
			io.Ouput(*mMemory);
			break;
		case CodeUnit::IptCur:
			*mMemory = io.Input();
			break;
		case CodeUnit::LoopBeg:
			if (*mMemory == 0)
			{
				szCodeIndex = curCode.szJmpIndex;//��ת
			}
			break;
		case CodeUnit::LoopEnd:
			szCodeIndex = curCode.szJmpIndex;//��������ת
			break;
		case CodeUnit::DbgInfo:
			printf("[Mem:%zu = 0x%02X]\n", mMemory.GetIndex(), *mMemory);
			break;
		default:
			printf("��������δ֪��ָ��\n");
			return false;//ע��ֱ�ӷ���ʧ��
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

	//TODO���ṩһЩ����debug�����ĺ������޸ġ���ʾ�ڴ棬����ִ�У��ϵ㣬�����ϵ㣬�ڴ�ϵ㣬�޸ġ���ʾָ���
};
