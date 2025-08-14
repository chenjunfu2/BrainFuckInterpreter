#pragma once

#include "FileStream.hpp"

#include <cstdint>
#include <vector>

struct CodeUnit//代码单元，执行器用
{
	/*
		BrainFuck

		符号		含义
		+		指针指向的字节的值加一
		-		指针指向的字节的值减一
		>		指针加一
		<		指针减一
		[		如果指针指向的单元值为零，向后跳转到对应的]指令的次一指令处
		]		向前跳转到对应的[指令处
		.		输出指针指向的单元内容（ASCⅡ码）
		,		输入内容到指针指向的单元（ASCⅡ码）

		下为新增
		?		输出当前内存单元格信息(地址(0基索引)+值(十六进制))
		#		到下一行为止都忽略
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

	//做完虚拟码编译后此处只能新增不能修改顺序
	enum Symbol : uint8_t
	{
		Unknown = (uint8_t)-1,//未知（正常无用）
		ProgEnd = 0,//特殊标记，程序结束
		AddCur,		//+
		SubCur,		//-
		NextMov,	//>
		PrevMov,	//<
		LoopBeg,	//[
		LoopEnd,	//]
		OptCur,		//.
		IptCur,		//,
		DbgInfo,	//?

		ZeroMem,	//内存置0，优化执行，不存在于代码中
	};

	Symbol enSymbol = Unknown;
	union
	{
		uint8_t u8CalcValue;//计算用累计值（注意：与内存单元，也就是uint8_t进行运算，能正确进行溢出环绕，属于可预见行为）
		size_t szMovOffset;//移动用偏移量
		size_t szJmpIndex = 0;//跳转用索引
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

	//获取活动成员，从enSymbol中
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

	bool IsOperator(void) const//判断是否是数值运算类型
	{
		return enSymbol == Symbol::AddCur || enSymbol == Symbol::SubCur;
	}

	bool IsPointerMove(void) const//判断是否是指针运算类型
	{
		return enSymbol == Symbol::NextMov || enSymbol == Symbol::PrevMov;
	}

	bool IsZeroMem(void) const//判断是否是内存置0操作
	{
		return enSymbol == Symbol::ZeroMem;
	}
	
public:
	bool Write(FileStream &fsWrite, bool bIsBigEndian = true) const
	{
		//Symbol是uint8，直接写
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
				//什么也不做
			}
			break;
		}

		return true;
	}

	bool Read(FileStream &fsRead, bool bIsBigEndian = true)
	{
		//u8直接读取
		if (!fsRead.ReadWithEndian(enSymbol, bIsBigEndian))
		{
			return false;
		}
		// 现在enSymbol已经在当前结构内，可以直接调用GetActivityMember判断
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
				//什么也不做
			}
			break;
		}


	}

	void PrintToStream(FileStream &fsPrint, const char *pPer = "", const char *pSuf = " ") const//FileStream可以打开标准流或文件，方便输出
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

//此方法可用于强制指定输出文件
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

//包装为标准io以便默认输出到标准io
static inline void PrintCodeList(const CodeList &listCode,
	const char *pHead = "", const char *pTail = "", const char *pPer = "", const char *pSuf = " ")
{
	FileStream fsPrint(stdout, false);
	PrintCodeList(listCode, fsPrint, pHead, pTail, pPer, pSuf);
}