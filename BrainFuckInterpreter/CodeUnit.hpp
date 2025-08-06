#pragma once

#include <cstdint>
#include <vector>

struct CodeUnit
{
	/*
		BrainFuck

		����		����
		>		ָ���һ
		<		ָ���һ
		+		ָ��ָ����ֽڵ�ֵ��һ
		-		ָ��ָ����ֽڵ�ֵ��һ
		.		���ָ��ָ��ĵ�Ԫ���ݣ�ASC���룩
		,		�������ݵ�ָ��ָ��ĵ�Ԫ��ASC���룩
		[		���ָ��ָ��ĵ�ԪֵΪ�㣬�����ת����Ӧ��]ָ��Ĵ�һָ�
		]		���ָ��ָ��ĵ�Ԫֵ��Ϊ�㣬��ǰ��ת����Ӧ��[ָ��Ĵ�һָ�

		��Ϊ����
		?		�����ǰ�ڴ浥Ԫ����Ϣ(��ַ+ֵ)
		#		����һ��Ϊֹ������
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
		uint8_t u8Val;//�������ۼ�ֵ
		size_t szIndex;//��ת������
	};
};

using CodeList = std::vector<CodeUnit>;