#pragma once

#include <conio.h>
#include <stdint.h>

class IO
{
public:
	uint8_t Input(void)
	{
		return (uint8_t)_getch();
	}

	void Ouput(uint8_t u8Char)
	{
		(void)_putch((int)u8Char);
	}
};