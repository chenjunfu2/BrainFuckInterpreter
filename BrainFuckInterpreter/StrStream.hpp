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
		//����
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

	operator bool(void) const//�������Ƿ����
	{
		return pStr != NULL;
	}

	bool Eof(void) const//�ж��Ƿ����
	{
		return bEof;
	}

	bool Error(void) const//�ж��Ƿ����
	{
		return false;
	}

	void ClearError(void)//������еĴ���
	{
		bEof = false;//ע����Ҫ����eof
		return;
	}

	void Rewind(void)//�ص���ͷ
	{
		bEof = false;
		szIndex = 0;
	}

	//��Ϊ����get���ж�eof�����Ի���ֵ�ǰ��ĩβ����ַ���ʱ����ǰ����eof
	//������Ҫ��GetChar�жϲ����ã�������ֱ����Eof�������ж�
	//������������һ���ַ������������أ�Ȼ������һ�ε��ú�
	//�˺�������eof��־λ�������������ַ�0������eof����true
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