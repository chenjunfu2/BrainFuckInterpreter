#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstdarg>

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

	//�ֽ���ת��

	//tTypeĿ������ pData���� bDataIsBigEndian���ݱ����˳���Ƿ�Ϊ����ֽ���
	//��������Ҫ��֤pData��С����Ϊsizeof(T)
	template<typename T>
	static void ConvertEndianData2Type(T &tType, const uint8_t *pData, bool bDataIsBigEndian)//ת���ֽ����ݵ�������
	{
		static constexpr size_t szTypeByte = sizeof(tType);

		if constexpr (szTypeByte == sizeof(uint8_t))//������ͱ������1�ֽ���ֱ�Ӹ�ֵ
		{
			tType = (T)pData[0];
		}
		else
		{
			tType = 0;//�ÿ�
			if (bDataIsBigEndian)
			{
				for (long i = 0; i < szTypeByte; ++i)//�������
				{
					tType <<= 8;//�ƶ�һ�ֽ�
					tType |= (T)pData[i];//��������ֽ�
				}
			}
			else
			{
				for (long i = szTypeByte - 1; i >= 0; --i)//�������
				{
					tType <<= 8;//�ƶ�һ�ֽ�
					tType |= (T)pData[i];//��������ֽ�
				}
			}
		}
	}

	//pDataĿ������ tType���� bDataIsBigEndian����������Ҫ��˳���Ƿ�Ϊ����ֽ���
	//��������Ҫ��֤pData��С����Ϊsizeof(T)
	template<typename T>
	static void ConvertEndianType2Data(uint8_t *pData, const T &tType, bool bDataNeedBigEndian)//ת������Ϊ�ֽ�����
	{
		constexpr size_t szTypeByte = sizeof(tType);

		if constexpr (szTypeByte == sizeof(uint8_t))//������ͱ������1�ֽ���ֱ�Ӹ�ֵ
		{
			pData[0] = (uint8_t)tType;
		}
		else
		{
			//������ʱ����
			T tTypeTemp = tType;
			if (bDataNeedBigEndian)
			{
				for (long i = szTypeByte - 1; i >= 0; --i)//�������
				{
					pData[i] = (uint8_t)tTypeTemp;//�ضϸ��ֽ�
					tTypeTemp >>= 8;//����һ�ֽ�
				}
			}
			else
			{
				for (long i = 0; i < szTypeByte; ++i)//�������
				{
					pData[i] = (uint8_t)tTypeTemp;//�ضϸ��ֽ�
					tTypeTemp >>= 8;//����һ�ֽ�
				}
			}
		}
	}

	template<typename T>
	bool ReadWithEndian(T &tData, bool bDataNeedBigEndian)
	{
		if constexpr (sizeof(T) == sizeof(uint8_t))
		{
			//ֱ�Ӷ�������
			if (Read(tData) != sizeof(T))
			{
				return false;
			}
		}
		else
		{
			//��ȡ����
			uint8_t u8ReadData[sizeof(T)];
			if (Read(u8ReadData) != sizeof(u8ReadData))
			{
				return false;
			}
			//ת���ֽ���
			ConvertEndianData2Type<T>(tData, u8ReadData, bDataNeedBigEndian);
		}

		return true;
	}

	template<typename T>
	bool WriteWithEndian(const T &tData, bool bDataNeedBigEndian)
	{
		if constexpr (sizeof(T) == sizeof(uint8_t))
		{
			//ֱ��д������
			if (Write(tData) != sizeof(T))
			{
				return false;
			}
		}
		else
		{
			//ת���ֽ���
			uint8_t u8WriteData[sizeof(T)];
			ConvertEndianType2Data<T>(u8WriteData, tData, bDataNeedBigEndian);

			//д������
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
		//�����������ÿ��Ԫ�ش�С����һ�ֽڵ�����ֱ�Ӷ������飬�����ÿ���ֽڵ��ֽ�����б任
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
		//�����������ÿ��Ԫ�ش�С����һ�ֽڵ�����ֱ��д���ļ��������ÿ���ֽڵ��ֽ�����б任
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

	//ת�����ã������Զ����С��ģ��
	template<typename T, size_t N>
	bool WriteWithEndian(const T(&tArr)[N], bool bDataNeedBigEndian)
	{
		WriteWithEndian(tArr, N, bDataNeedBigEndian);
	}

	//ת�����ã������Զ����С��ģ��
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