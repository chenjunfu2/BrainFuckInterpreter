#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstdint>

class FileStream
{
private:
	FILE *pFile = NULL;
public:
	FileStream(void) = default;

	FileStream(const FileStream &) = delete;
	FileStream(FileStream &&_Move) :pFile(_Move.pFile)
	{
		_Move.pFile = NULL;
	}

	FileStream(FILE *_pFile) :pFile(_pFile)
	{}

	FileStream(char const *_FileName, char const *_Mode)
	{
		Open(_FileName, _Mode);
	}

	~FileStream(void)
	{
		Close();
	}

	FileStream &operator =(const FileStream &) = delete;
	FileStream &operator =(FileStream &&_Move)
	{
		pFile = _Move.pFile;
		_Move.pFile = NULL;
	}

	void Open(char const *_FileName, char const *_Mode)
	{
		Close();
		pFile = fopen(_FileName, _Mode);
	}

	void Close(void)
	{
		if (pFile != NULL)
		{
			free(pFile);
			pFile = NULL;
		}
	}

	operator bool(void)
	{
		return pFile != NULL;
	}

	FILE *GetFile(void)
	{
		return pFile;
	}

	void SetFile(FILE *pNewFile)
	{
		Close();
		pFile = pNewFile;
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

	int SetFilePos(int64_t _Offset)
	{
		return _fseeki64(pFile, _Offset, SEEK_SET);
	}

	int MovFilePos(int64_t _Offset)
	{
		return _fseeki64(pFile, _Offset, SEEK_CUR);
	}

	int EndFilePos(int64_t _Offset)
	{
		return _fseeki64(pFile, _Offset, SEEK_END);
	}

	bool Eof(void)
	{
		return feof(pFile);
	}

	int64_t GetFileSize(void)
	{
		int64_t i64CurPos = GetFilePos();//获取当前偏移以便恢复
		EndFilePos(0);//设置到文件末尾

		int64_t i64FileSize = GetFilePos();//再次获取偏移作为文件大小
		SetFilePos(i64CurPos);//恢复回原先的偏移量

		return i64FileSize;
	}
};