#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstdint>

class FileStream
{
private:
	FILE *pFile = NULL;//默认为NULL
	bool bClose = false;//默认不持有对象（不关闭）
public:
	FileStream(void) = default;

	FileStream(const FileStream &) = delete;
	FileStream(FileStream &&_Move) :
		pFile(_Move.pFile), bClose(_Move.bClose)
	{
		_Move.pFile = NULL;
		_Move.bClose = false;
	}

	FileStream(FILE *_pFile, bool _bClose = false) ://从file来的对象默认不持有
		pFile(_pFile), bClose(_bClose)
	{}

	FileStream(char const *_FileName, char const *_Mode, bool _bClose = true)//打开的对象默认持有
	{
		Open(_FileName, _Mode, _bClose);//这里千万不可在构造提前初始化bClose，否则可能误关闭原先的对象
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

	void Open(char const *_FileName, char const *_Mode, bool _bClose = true)//打开的对象默认持有
	{
		Close();//先关闭原先的（如果可以）
		//再打开并赋值
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

		//不论是否关闭，强制为NULL
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

	void SetFile(FILE *pNewFile, bool _bClose = false)//默认不持有
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
		return _fseeki64(pFile, _Offset, SEEK_SET) == 0;//成功返回0
	}

	bool MovFilePos(int64_t _Offset)
	{
		return _fseeki64(pFile, _Offset, SEEK_CUR) == 0;//成功返回0
	}

	bool EndFilePos(int64_t _Offset)
	{
		return _fseeki64(pFile, _Offset, SEEK_END) == 0;//成功返回0
	}

	bool Eof(void) const//判断是否文件尾
	{
		return feof(pFile) != 0;//返回非0则文件尾，否则返回0
	}

	bool Error(void) const//判断是否产生错误
	{
		return ferror(pFile) != 0;//返回非0则错误，否则返回0
	}

	void ClearError(void)//清除流中的错误
	{
		clearerr(pFile);
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