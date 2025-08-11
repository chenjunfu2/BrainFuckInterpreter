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
		?		�����ǰ�ڴ浥Ԫ����Ϣ(��ַ(0������)+ֵ(ʮ������))
		#		����һ��Ϊֹ������
	*/
	static const inline char *BfCodeChar[] =
	{
		"END",
		">",
		"<",
		"+",
		"-",
		".",
		",",
		"[",
		"]",
		"?",
		"ZRM",
	};

	enum Symbol : uint8_t
	{
		Unknown = -1,//δ֪���������ã�
		ProgEnd = 0,//�����ǣ��������
		NextMov,	//>
		PrevMov,	//<
		AddCur,		//+
		SubCur,		//-
		OptCur,		//.
		IptCur,		//,
		LoopBeg,	//[
		LoopEnd,	//]
		DbgInfo,	//?

		ZeroMem,	//�ڴ���0���Ż�ִ�У��������ڴ�����
	};

	Symbol enSymbol = Unknown;
	union
	{
		uint8_t u8CalcValue = 0;//�������ۼ�ֵ��ע�⣺���ڴ浥Ԫ��Ҳ����uint8_t�������㣬����ȷ����������ƣ����ڿ�Ԥ����Ϊ��
		size_t szJmpIndex;//��ת������
		size_t szMovOffset;//�ƶ���ƫ����
	};
};

using CodeList = std::vector<CodeUnit>;