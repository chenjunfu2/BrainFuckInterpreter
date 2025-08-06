#pragma once

#include <cstdint>
#include <vector>

struct CodeUnit
{
	/*
		BrainFuck

		符号		含义
		>		指针加一
		<		指针减一
		+		指针指向的字节的值加一
		-		指针指向的字节的值减一
		.		输出指针指向的单元内容（ASCⅡ码）
		,		输入内容到指针指向的单元（ASCⅡ码）
		[		如果指针指向的单元值为零，向后跳转到对应的]指令的次一指令处
		]		如果指针指向的单元值不为零，向前跳转到对应的[指令的次一指令处

		下为新增
		?		输出当前内存单元格信息(地址(0基索引)+值(十六进制))
		#		到下一行为止都忽略
	*/
	enum Symbol : uint8_t
	{
		Unknown = 0,//未知（正常无用）
		ProgEnd = 0,//特殊标记，程序结束
		NextMov,	//>
		PrevMov,	//<
		AddCur,		//+
		SubCur,		//-
		OptCur,		//.
		IptCur,		//,
		LoopBeg,	//[
		LoopEnd,	//]
		DbgInfo,	//?
	};

	Symbol enSymbol = Unknown;
	union
	{
		size_t szCalcValue = 0;//计算用累计值（注意：与内存单元，也就是uint8_t进行运算，能正确进行溢出环绕，属于可预见行为）
		size_t szJmpIndex;//跳转用索引
		size_t szMovOffset;//移动用偏移量
	};
};

using CodeList = std::vector<CodeUnit>;