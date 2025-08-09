#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstdint>

class FileStream
{
private:
	FILE *pFile = NULL;//Ĭ��ΪNULL
	bool bClose = false;//Ĭ�ϲ����ж��󣨲��رգ�
public:
	FileStream(void) = default;

	FileStream(const FileStream &) = delete;
	FileStream(FileStream &&_Move) :
		pFile(_Move.pFile), bClose(_Move.bClose)
	{
		_Move.pFile = NULL;
		_Move.bClose = false;
	}

	FileStream(FILE *_pFile, bool _bClose = false) ://��file���Ķ���Ĭ�ϲ�����
		pFile(_pFile), bClose(_bClose)
	{}

	FileStream(char const *_FileName, char const *_Mode, bool _bClose = true)//�򿪵Ķ���Ĭ�ϳ���
	{
		Open(_FileName, _Mode, _bClose);//����ǧ�򲻿��ڹ�����ǰ��ʼ��bClose�����������ر�ԭ�ȵĶ���
	}

	~FileStream(void)
	{
		Close();
	}

	FileStream &operator =(const FileStream &) = delete;
	FileStream &operator =(FileStream &&_Move)
	{
		pFile = _Move.pFile;
		bClose = _Move.bClose;

		_Move.pFile = NULL;
		_Move.bClose = false;
	}

	void Open(char const *_FileName, char const *_Mode, bool _bClose = true)//�򿪵Ķ���Ĭ�ϳ���
	{
		Close();//�ȹر�ԭ�ȵģ�������ԣ�
		//�ٴ򿪲���ֵ
		pFile = fopen(_FileName, _Mode);
		bClose = _bClose;
	}

	void Close(void)
	{
		if (bClose == true &&
			pFile != NULL)
		{
			fclose(pFile);
		}

		//�����Ƿ�رգ�ǿ��ΪNULL
		pFile = NULL;
	}

	operator bool(void) const
	{
		return pFile != NULL;
	}

	FILE *GetFile(void)
	{
		return pFile;
	}

	void SetFile(FILE *pNewFile, bool _bClose = false)//Ĭ�ϲ�����
	{
		Close();
		pFile = pNewFile;
	}

	char GetChar(void)
	{
		char cRead;
		Read(cRead);
		return cRead;
	}

	bool PutChar(char cWrite)
	{
		return Write(cWrite) == sizeof(cWrite);
	}

	bool UnGet(void)
	{
		return MovFilePos(-1);
	}

	size_t Read(void *_Buffer, size_t _Size)
	{
		return fread(_Buffer, 1, _Size, pFile);
	}

	template<typename T>
	size_t Read(T &_Buffer)
	{
		return Read(&_Buffer, sizeof(_Buffer));
	}

	template<typename T, size_t S>
	size_t Read(T(&_Buffer)[S])
	{
		return Read(&_Buffer, sizeof(_Buffer));
	}
	
	size_t Write(const void *_Buffer, size_t _Size)
	{
		return fwrite(_Buffer, 1, _Size, pFile);
	}

	template<typename T>
	size_t Write(const T &_Buffer)
	{
		return Write(&_Buffer, sizeof(_Buffer));
	}

	template<typename T, size_t S>
	size_t Write(const T(&_Buffer)[S])
	{
		return Write(&_Buffer, sizeof(_Buffer));
	}

	int64_t GetFilePos(void)
	{
		return _ftelli64(pFile);
	}

	void Rewind(void)
	{
		rewind(pFile);
	}

	bool SetFilePos(int64_t _Offset)
	{
		return _fseeki64(pFile, _Offset, SEEK_SET) == 0;//�ɹ�����0
	}

	bool MovFilePos(int64_t _Offset)
	{
		return _fseeki64(pFile, _Offset, SEEK_CUR) == 0;//�ɹ�����0
	}

	bool EndFilePos(int64_t _Offset)
	{
		return _fseeki64(pFile, _Offset, SEEK_END) == 0;//�ɹ�����0
	}

	bool Eof(void) const//�ж��Ƿ��ļ�β
	{
		return feof(pFile) != 0;//���ط�0���ļ�β�����򷵻�0
	}

	bool Error(void) const//�ж��Ƿ��������
	{
		return ferror(pFile) != 0;//���ط�0����󣬷��򷵻�0
	}

	void ClearError(void)//������еĴ���
	{
		clearerr(pFile);
	}

	int64_t GetFileSize(void)
	{
		int64_t i64CurPos = GetFilePos();//��ȡ��ǰƫ���Ա�ָ�
		EndFilePos(0);//���õ��ļ�ĩβ

		int64_t i64FileSize = GetFilePos();//�ٴλ�ȡƫ����Ϊ�ļ���С
		SetFilePos(i64CurPos);//�ָ���ԭ�ȵ�ƫ����

		return i64FileSize;
	}
};