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
		?		输出当前内存单元格信息(地址+值)
		#		到下一行为止都忽略
	*/
	enum Symbol : uint8_t
	{
		Unknown = 0,
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

	Symbol sym;
	union
	{
		uint8_t u8Val;//计算用累计值
		size_t szIndex;//跳转用索引
	};
};

using CodeList = std::vector<CodeUnit>;