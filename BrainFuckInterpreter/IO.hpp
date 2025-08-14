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
		NoBuffer = 0,//无缓冲io（默认模式）
		LineBuffer,//行缓冲io（控制台输入提交前可回显、修改，输出需要以行为结尾）
		FileBuffer,//文件缓冲io（与文件交互输入输出）
	};

	//直接公开成员

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
			MyAssert(false, "IO错误：错误的输入缓冲类型");
			break;
		}

		//能执行到这里也是没谁了
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
			MyAssert(false, "IO错误：错误的输出缓冲类型");
			break;
		}
	}
};