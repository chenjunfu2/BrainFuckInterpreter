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

	size_t Write(void *_Buffer, size_t _Size)
	{
		return fwrite(_Buffer, 1, _Size, pFile);
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
};