#pragma once

#include "FileStream.hpp"

#include <cstdint>
#include <vector>

struct CodeUnit//���뵥Ԫ��ִ������
{
	/*
		BrainFuck

		����		����
		+		ָ��ָ����ֽڵ�ֵ��һ
		-		ָ��ָ����ֽڵ�ֵ��һ
		>		ָ���һ
		<		ָ���һ
		[		���ָ��ָ��ĵ�ԪֵΪ�㣬�����ת����Ӧ��]ָ��Ĵ�һָ�
		]		��ǰ��ת����Ӧ��[ָ�
		.		���ָ��ָ��ĵ�Ԫ���ݣ�ASC���룩
		,		�������ݵ�ָ��ָ��ĵ�Ԫ��ASC���룩

		��Ϊ����
		?		�����ǰ�ڴ浥Ԫ����Ϣ(��ַ(0������)+ֵ(ʮ������))
		#		����һ��Ϊֹ������
	*/
	static const inline char *BfCodeChar[] =
	{
		"END",
		"+",
		"-",
		">",
		"<",
		"[",
		"]",
		".",
		",",
		"?",
		"ZRM",
	};

	//��������������˴�ֻ�����������޸�˳��
	enum Symbol : uint8_t
	{
		Unknown = (uint8_t)-1,//δ֪���������ã�
		ProgEnd = 0,//�����ǣ��������
		AddCur,		//+
		SubCur,		//-
		NextMov,	//>
		PrevMov,	//<
		LoopBeg,	//[
		LoopEnd,	//]
		OptCur,		//.
		IptCur,		//,
		DbgInfo,	//?

		ZeroMem,	//�ڴ���0���Ż�ִ�У��������ڴ�����
	};

	Symbol enSymbol = Unknown;
	union
	{
		uint8_t u8CalcValue;//�������ۼ�ֵ��ע�⣺���ڴ浥Ԫ��Ҳ����uint8_t�������㣬����ȷ����������ƣ����ڿ�Ԥ����Ϊ��
		size_t szMovOffset;//�ƶ���ƫ����
		size_t szJmpIndex = 0;//��ת������
	};
public:
	enum Activity : uint8_t
	{
		ACT_Unknown = (uint8_t)-1,
		ACT_NULL = 0,
		ACT_u8CalcValue,
		ACT_szMovOffset,
		ACT_szJmpIndex,
	};

	//��ȡ���Ա����enSymbol��
	Activity GetActivityMember(void) const
	{
		switch (enSymbol)
		{
		case Symbol::AddCur:
		case Symbol::SubCur:
			{
				return ACT_u8CalcValue;
			}
			break;
		case Symbol::NextMov:
		case Symbol::PrevMov:
			{
				return ACT_szMovOffset;
			}
			break;
		case Symbol::LoopBeg:
		case Symbol::LoopEnd:
			{
				return ACT_szJmpIndex;
			}
			break;
		case Symbol::ProgEnd:
		case Symbol::OptCur:
		case Symbol::IptCur:
		case Symbol::DbgInfo:
		case Symbol::ZeroMem:
			{
				return ACT_NULL;
			}
			break;
		case Symbol::Unknown:
		default:
			{
				return ACT_Unknown;
			}
			break;
		}

		return ACT_Unknown;
	}

	bool IsOperator(void) const//�ж��Ƿ�����ֵ��������
	{
		return enSymbol == Symbol::AddCur || enSymbol == Symbol::SubCur;
	}

	bool IsPointerMove(void) const//�ж��Ƿ���ָ����������
	{
		return enSymbol == Symbol::NextMov || enSymbol == Symbol::PrevMov;
	}

	bool IsZeroMem(void) const//�ж��Ƿ����ڴ���0����
	{
		return enSymbol == Symbol::ZeroMem;
	}
	
public:
	bool Write(FileStream &fsWrite, bool bIsBigEndian = true) const
	{
		//Symbol��uint8��ֱ��д
		if (!fsWrite.WriteWithEndian(enSymbol, bIsBigEndian))
		{
			return false;
		}
		
		Activity actMemb = GetActivityMember();
		switch (actMemb)
		{
		case Activity::ACT_u8CalcValue:
			{
				if (!fsWrite.WriteWithEndian(u8CalcValue, bIsBigEndian))
				{
					return false;
				}
			}
			break;
		case Activity::ACT_szMovOffset:
			{
				if (fsWrite.WriteWithEndian(szMovOffset, bIsBigEndian))
				{
					return false;
				}
			}
			break;
		case Activity::ACT_szJmpIndex:
			{
				if (fsWrite.WriteWithEndian(szJmpIndex, bIsBigEndian))
				{
					return false;
				}
			}
			break;
		case Activity::ACT_Unknown:
		case Activity::ACT_NULL:
		default:
			{
				//ʲôҲ����
			}
			break;
		}

		return true;
	}

	bool Read(FileStream &fsRead, bool bIsBigEndian = true)
	{
		//u8ֱ�Ӷ�ȡ
		if (!fsRead.ReadWithEndian(enSymbol, bIsBigEndian))
		{
			return false;
		}
		// ����enSymbol�Ѿ��ڵ�ǰ�ṹ�ڣ�����ֱ�ӵ���GetActivityMember�ж�
		Activity actMemb = GetActivityMember();
		switch (actMemb)
		{
		case Activity::ACT_u8CalcValue:
			{
				if (!fsRead.ReadWithEndian(u8CalcValue, bIsBigEndian))
				{
					return false;
				}
			}
			break;
		case Activity::ACT_szMovOffset:
			{
				if (!fsRead.ReadWithEndian(szMovOffset, bIsBigEndian))
				{
					return false;
				}
			}
			break;
		case Activity::ACT_szJmpIndex:
			{
				if (fsRead.ReadWithEndian(szJmpIndex, bIsBigEndian))
				{
					return false;
				}
			}
			break;
		case Activity::ACT_Unknown:
		case Activity::ACT_NULL:
		default:
			{
				//ʲôҲ����
			}
			break;
		}


	}

	void PrintToStream(FileStream &fsPrint, const char *pPer = "", const char *pSuf = " ") const//FileStream���Դ򿪱�׼�����ļ����������
	{
		Activity actMemb = GetActivityMember();
		switch (actMemb)
		{
		case Activity::ACT_u8CalcValue:
			{
				fsPrint.Print("%s%s%u%s", pPer, BfCodeChar[enSymbol], u8CalcValue, pSuf);
			}
			break;
		case Activity::ACT_szMovOffset:
			{
				fsPrint.Print("%s%s%zu%s", pPer, BfCodeChar[enSymbol], szMovOffset, pSuf);
			}
			break;
		case Activity::ACT_szJmpIndex:
			{
				fsPrint.Print("%s%s%zu%s", pPer, BfCodeChar[enSymbol], szJmpIndex, pSuf);
			}
			break;
		case Activity::ACT_NULL:
			{
				fsPrint.Print("%s%s%s", pPer, BfCodeChar[enSymbol], pSuf);
			}
			break;
		case Activity::ACT_Unknown:
		default:
			{
				fsPrint.Print("UNNOWN ");
			}
			break;
		}
	}
};

using CodeList = std::vector<CodeUnit>;

//�˷���������ǿ��ָ������ļ�
static inline void PrintCodeList(const CodeList &listCode, FileStream &fsPrint,
	const char *pHead = "", const char *pTail = "", const char *pPer = "", const char *pSuf = " ")
{
	fsPrint.Print("%s", pHead);

	for (const auto &it : listCode)
	{
		it.PrintToStream(fsPrint, pPer, pSuf);
	}

	fsPrint.Print("%s", pTail);
}

//��װΪ��׼io�Ա�Ĭ���������׼io
static inline void PrintCodeList(const CodeList &listCode,
	const char *pHead = "", const char *pTail = "", const char *pPer = "", const char *pSuf = " ")
{
	FileStream fsPrint(stdout, false);
	PrintCodeList(listCode, fsPrint, pHead, pTail, pPer, pSuf);
}