#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstdarg>

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

	//字节序转换

	//tType目标类型 pData数据 bDataIsBigEndian数据本身的顺序是否为大端字节序
	//调用者需要保证pData大小至少为sizeof(T)
	template<typename T>
	static void ConvertEndianData2Type(T &tType, const uint8_t *pData, bool bDataIsBigEndian)//转换字节数据到类型内
	{
		static constexpr size_t szTypeByte = sizeof(tType);

		if constexpr (szTypeByte == sizeof(uint8_t))//如果类型本身就是1字节则直接赋值
		{
			tType = (T)pData[0];
		}
		else
		{
			tType = 0;//置空
			if (bDataIsBigEndian)
			{
				for (long i = 0; i < szTypeByte; ++i)//正序遍历
				{
					tType <<= 8;//移动一字节
					tType |= (T)pData[i];//放在最低字节
				}
			}
			else
			{
				for (long i = szTypeByte - 1; i >= 0; --i)//倒序遍历
				{
					tType <<= 8;//移动一字节
					tType |= (T)pData[i];//放在最高字节
				}
			}
		}
	}

	//pData目标数据 tType类型 bDataIsBigEndian数据最终需要的顺序是否为大端字节序
	//调用者需要保证pData大小至少为sizeof(T)
	template<typename T>
	static void ConvertEndianType2Data(uint8_t *pData, const T &tType, bool bDataNeedBigEndian)//转换类型为字节数据
	{
		constexpr size_t szTypeByte = sizeof(tType);

		if constexpr (szTypeByte == sizeof(uint8_t))//如果类型本身就是1字节则直接赋值
		{
			pData[0] = (uint8_t)tType;
		}
		else
		{
			//保存临时变量
			T tTypeTemp = tType;
			if (bDataNeedBigEndian)
			{
				for (long i = szTypeByte - 1; i >= 0; --i)//倒序遍历
				{
					pData[i] = (uint8_t)tTypeTemp;//截断高字节
					tTypeTemp >>= 8;//右移一字节
				}
			}
			else
			{
				for (long i = 0; i < szTypeByte; ++i)//正序遍历
				{
					pData[i] = (uint8_t)tTypeTemp;//截断高字节
					tTypeTemp >>= 8;//右移一字节
				}
			}
		}
	}

	template<typename T>
	bool ReadWithEndian(T &tData, bool bDataNeedBigEndian)
	{
		if constexpr (sizeof(T) == sizeof(uint8_t))
		{
			//直接读入数据
			if (Read(tData) != sizeof(T))
			{
				return false;
			}
		}
		else
		{
			//读取数据
			uint8_t u8ReadData[sizeof(T)];
			if (Read(u8ReadData) != sizeof(u8ReadData))
			{
				return false;
			}
			//转换字节序
			ConvertEndianData2Type<T>(tData, u8ReadData, bDataNeedBigEndian);
		}

		return true;
	}

	template<typename T>
	bool WriteWithEndian(const T &tData, bool bDataNeedBigEndian)
	{
		if constexpr (sizeof(T) == sizeof(uint8_t))
		{
			//直接写入数据
			if (Write(tData) != sizeof(T))
			{
				return false;
			}
		}
		else
		{
			//转换字节序
			uint8_t u8WriteData[sizeof(T)];
			ConvertEndianType2Data<T>(u8WriteData, tData, bDataNeedBigEndian);

			//写入数据
			if (Write(u8WriteData) != sizeof(u8WriteData))
			{
				return false;
			}
		}

		return true;
	}

	template<typename T>
	bool ReadWithEndian(T *tArr, size_t szArrLen, bool bDataNeedBigEndian)
	{
		//两种情况处理，每个元素大小等于一字节的数组直接读入数组，否则对每个字节的字节序进行变换
		if constexpr (sizeof(T) == sizeof(uint8_t))
		{
			if (Read(tArr, szArrLen * sizeof(T)) != szArrLen * sizeof(T))
			{
				return false;
			}
		}
		else
		{
			for (size_t i = 0; i < szArrLen; ++i)
			{
				if (!ReadWithEndian<T>(tArr[i], bDataNeedBigEndian))
				{
					return false;
				}
			}
		}

		return true;
	}

	template<typename T>
	bool WriteWithEndian(const T *tArr, size_t szArrLen, bool bDataNeedBigEndian)
	{
		//两种情况处理，每个元素大小等于一字节的数组直接写入文件，否则对每个字节的字节序进行变换
		if constexpr (sizeof(T) == sizeof(uint8_t))
		{
			if (Write(tArr, szArrLen * sizeof(T)) != szArrLen * sizeof(T))
			{
				return false;
			}
		}
		else
		{
			for (size_t i = 0; i < szArrLen; ++i)
			{
				if (!WriteWithEndian<T>(tArr[i], bDataNeedBigEndian))
				{
					return false;
				}
			}
		}

		return true;
	}

	//转发调用，数组自动求大小套模板
	template<typename T, size_t N>
	bool WriteWithEndian(const T(&tArr)[N], bool bDataNeedBigEndian)
	{
		WriteWithEndian(tArr, N, bDataNeedBigEndian);
	}

	//转发调用，数组自动求大小套模板
	template<typename T, size_t N>
	bool ReadWithEndian(T(&tArr)[N], bool bDataNeedBigEndian)
	{
		ReadWithEndian(tArr, N, bDataNeedBigEndian);
	}

	int Print(_Printf_format_string_ const char *pFormat, ...)
	{
		va_list vl;
		va_start(vl, pFormat);
		int ret = vfprintf(pFile, pFormat, vl);
		va_end(vl);

		return ret;
	}

};