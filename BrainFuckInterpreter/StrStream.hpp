#pragma once

#include <stddef.h>

class StrStream
{
private:
	const char *pStr = NULL;
	bool bEof = false;
	size_t szSize = 0;
	size_t szIndex = 0;
public:
	StrStream(const char *_pStr = NULL, size_t _szSize = 0, size_t _szIndex = 0) : pStr(_pStr), bEof(false), szSize(_szSize), szIndex(_szIndex)
	{}
	~StrStream(void) = default;

	StrStream(const StrStream &) = delete;
	StrStream(StrStream &&_Move) noexcept :
		pStr(_Move.pStr),
		bEof(_Move.bEof),
		szSize(_Move.szSize),
		szIndex(_Move.szIndex)
	{
		//重置
		_Move.pStr = NULL;
		_Move.bEof = false;
		_Move.szSize = 0;
		_Move.szIndex = 0;
	}
	
	StrStream &operator=(const StrStream &) = delete;
	StrStream &operator=(StrStream &&_Move) noexcept
	{
		pStr = _Move.pStr;
		bEof = _Move.bEof;
		szSize = _Move.szSize;
		szIndex = _Move.szIndex;

		_Move.pStr = NULL;
		_Move.bEof = false;
		_Move.szSize = 0;
		_Move.szIndex = 0;

		return *this;
	}

	const char *GetStr(void)
	{
		return pStr;
	}

	void SetStr(const char *_pStr = NULL, size_t _szSize = 0, size_t _szIndex = 0)
	{
		bEof = false;
		pStr = _pStr;
		szSize = _szSize;
		szIndex = _szIndex;
	}

	void SetIndex(size_t _szIndex)
	{
		bEof = false;
		szIndex = _szIndex;
	}

	size_t GetIndex(void)
	{
		return szIndex;
	}

	operator bool(void) const//测试流是否可用
	{
		return pStr != NULL;
	}

	bool Eof(void) const//判断是否结束
	{
		return bEof;
	}

	bool Error(void) const//判断是否错误
	{
		return false;
	}

	void ClearError(void)//清除流中的错误
	{
		bEof = false;//注意需要清理eof
		return;
	}

	void Rewind(void)//回到开头
	{
		bEof = false;
		szIndex = 0;
	}

	//因为是先get再判断eof，所以会出现当前是末尾最后字符的时候提前返回eof
	//所以需要在GetChar判断并设置，而不是直接在Eof函数内判断
	//这样如果是最后一个字符则能正常返回，然后在下一次调用后
	//此函数设置eof标志位，并返回虚拟字符0，接着eof返回true
	char GetChar(void)
	{
		if (szIndex >= szSize)
		{
			bEof = true;
			return '\0';
		}

		return pStr[szIndex++];
	}

	bool UnGet(void)
	{
		--szIndex;
		return true;
	}
};