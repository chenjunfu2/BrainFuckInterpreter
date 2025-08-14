#pragma once

#include "MyAssert.hpp"
#include "FileStream.hpp"

#include <cstdio>
#include <conio.h>
#include <cstdint>

class IO
{
public:
	enum IOType :uint8_t
	{
		Unknown = (uint8_t)-1,
		NoBuffer = 0,//�޻���io��Ĭ��ģʽ��
		LineBuffer,//�л���io������̨�����ύǰ�ɻ��ԡ��޸ģ������Ҫ����Ϊ��β��
		FileBuffer,//�ļ�����io�����ļ��������������
	};

	//ֱ�ӹ�����Ա

	IOType typeInput = IOType::NoBuffer;
	IOType typeOutput = IOType::NoBuffer;

	FileStream fsInputBuffer = {};
	FileStream fsOutputBuffer = {};
public:
	IO(void) = default;
	~IO(void) = default;

	IO(IO &&) = default;
	IO &operator = (IO &&) = default;

	IO(const IO &) = delete;
	IO &operator =(const IO &) = delete;

	uint8_t Input(void)
	{
		switch (typeInput)
		{
		case IO::NoBuffer:
			return (uint8_t)_getch();
			break;
		case IO::LineBuffer:
			return (uint8_t)getchar();
			break;
		case IO::FileBuffer:
			return (uint8_t)fsInputBuffer.GetChar();
			break;
		case IO::Unknown:
		default:
			MyAssert(false, "IO���󣺴�������뻺������");
			break;
		}

		//��ִ�е�����Ҳ��û˭��
		return '\0';
	}

	void Ouput(uint8_t u8Char)
	{
		switch (typeInput)
		{
		case IO::NoBuffer:
			(void)_putch((int)u8Char);
			break;
		case IO::LineBuffer:
			putchar((int)u8Char);
			break;
		case IO::FileBuffer:
			fsInputBuffer.PutChar((char)u8Char);
			break;
		case IO::Unknown:
		default:
			MyAssert(false, "IO���󣺴���������������");
			break;
		}
	}
};