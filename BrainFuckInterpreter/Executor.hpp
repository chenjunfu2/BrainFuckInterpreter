#pragma once

#include "CodeUnit.hpp"
#include "MemoryManager.hpp"
#include "IO.hpp"
#include "MyAssert.hpp"

#include <vector>

class Executor//ִ����
{
private:
	size_t szCodeListIP = 0;//ָ��ָ��

	IO io{};
	CodeList listCode{};
	MemoryManager mMemory{};
public:
	Executor(void) = default;
	Executor(CodeList _listCode, size_t _szCodeListIP = 0, IO _io = {}, MemoryManager _mMemory = {}) :szCodeListIP(_szCodeListIP), io(std::move(_io)), listCode(std::move(_listCode)), mMemory(std::move(_mMemory))
	{}
	~Executor(void) = default;

	Executor(const Executor &&) = delete;
	Executor(Executor &&_Move) :szCodeListIP(std::move(_Move.szCodeListIP)), io(std::move(_Move.io)), listCode(std::move(_Move.listCode)), mMemory(std::move(_Move.mMemory))
	{
		_Move.szCodeListIP = 0;//�����Ա��Ҫ�ֶ���������
	}

	Executor &operator=(const Executor &) = delete;
	Executor &operator=(Executor &&_Move)
	{
		szCodeListIP = std::move(_Move.szCodeListIP);
		_Move.szCodeListIP = 0;//�����Ա��Ҫ�ֶ���������

		//���Ա�������ƶ���ֵ����
		io = std::move(_Move.io);
		listCode = std::move(_Move.listCode);
		mMemory = std::move(_Move.mMemory);
	}

	void SetIO(IO _io)
	{
		io = std::move(_io);
	}

	void SetMemory(MemoryManager _mMemory)
	{
		mMemory = std::move(_mMemory);
	}

	void SetListCode(CodeList _listCode, size_t _szCodeListIP = 0)
	{
		szCodeListIP = _szCodeListIP;
		listCode = std::move(_listCode);
	}

	void ResetMemory(void)
	{
		mMemory.Reset();
	}

	const IO &GetIO(void)
	{
		return io;
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
		return szCodeListIP;
	}

	bool Once(void)//����true�����ܼ���ִ�У�false����ִ�н���
	{
		if (szCodeListIP >= listCode.size())
		{
			printf("��������ָ��ָ��[%zu]����������Χ[0~%zu)\n", szCodeListIP, listCode.size());
			exit(-1);
		}

		auto &curCode = listCode[szCodeListIP++];
		switch (curCode.enSymbol)
		{
		case CodeUnit::ProgEnd://������־
			return false;//ֱ�ӽ���
			break;
		case CodeUnit::AddCur:
			*mMemory += curCode.u8CalcValue;
			break;
		case CodeUnit::SubCur:
			*mMemory -= curCode.u8CalcValue;
			break;
		case CodeUnit::NextMov:
			mMemory += curCode.szMovOffset;
			break;
		case CodeUnit::PrevMov:
			mMemory -= curCode.szMovOffset;
			break;
		case CodeUnit::LoopBeg:
			if (*mMemory == 0)
			{
				szCodeListIP = curCode.szJmpIndex;//������ת������ָ��ָ��
			}
			break;
		case CodeUnit::LoopEnd:
			szCodeListIP = curCode.szJmpIndex;//��������ת������ָ��ָ��
			break;
		case CodeUnit::OptCur:
			io.Ouput(*mMemory);
			break;
		case CodeUnit::IptCur:
			*mMemory = io.Input();
			break;
		case CodeUnit::DbgInfo:
			printf("[Mem:%zu = 0x%02X]\n", mMemory.GetIndex(), *mMemory);
			break;
		case CodeUnit::Symbol::ZeroMem:
			*mMemory = 0;//�Ż�ִ�У��ڴ���0
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
