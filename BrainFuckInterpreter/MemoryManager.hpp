#pragma once

#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstdio>

class MemoryManager//内存管理器
{
private:
	uint8_t *pBase = NULL;//基地址指针
	size_t szIndex = 0;//当前位置下标
	size_t szSize = 0;//数组总大小
private:
	constexpr const static size_t DEFAULT_EXPAND_FACTOR = 2;//默认二倍

	void Clean(void)//清理，注意会删掉内存！
	{
		free(pBase);
		pBase = NULL;
		szIndex = 0;
		szSize = 0;
	}

	bool IsMulOverflow(size_t szLeft, size_t szRight)//szRight写较少改变的变量或常量性能更优
	{
		return szLeft > (SIZE_MAX / szRight);
	}

	void MemAlloc(void)//构造用，勿在其余位置使用
	{
		pBase = (uint8_t *)malloc(szSize);
		if (pBase == NULL)
		{
			Clean();
			printf("内存分配失败");
			exit(0);
		}
		memset(pBase, 0, szSize);//置零
	}

	void CheckExpand(size_t szMoveSize)
	{
		if (szIndex + szMoveSize >= szSize)
		{
			Expand(szIndex + szMoveSize);
		}
	}

	void Expand(size_t szNewSize)//扩容
	{
		if (szNewSize <= szSize)//比当前还小，扩容个p
		{
			return;
		}

		//哪怕szNewSize比当前szSize大于1都2倍扩容
		size_t szExpandFactor = DEFAULT_EXPAND_FACTOR;
		while (szSize * szExpandFactor < szNewSize)
		{
			//计算前检测
			if (IsMulOverflow(szExpandFactor, DEFAULT_EXPAND_FACTOR))
			{
				Clean();
				printf("内存扩容失败：类型溢出\n");
				exit(-1);
			}

			szExpandFactor *= DEFAULT_EXPAND_FACTOR;//倍增直到大于或溢出

			//计算前检测
			if (IsMulOverflow(szExpandFactor, szSize))
			{
				Clean();
				printf("内存扩容失败：类型溢出\n");
				exit(-1);
			}
		}

		//重分配
		uint8_t *pNewBase = (uint8_t *)realloc(pBase, szSize * szExpandFactor);
		if (pNewBase == NULL)
		{
			Clean();
			printf("内存扩容失败：内存不足\n");
			exit(-1);
		}
		//扩容成功替换指针
		pBase = pNewBase;
		//新位置（后半段）置零
		memset(&pBase[szSize], 0, szSize);
		szSize *= szExpandFactor;
	}
public:
	MemoryManager(const MemoryManager &) = delete;//禁止拷贝
	MemoryManager &operator=(const MemoryManager &) = delete;//禁止赋值

	MemoryManager(MemoryManager &&_Move) :
		pBase(_Move.pBase),
		szIndex(_Move.szIndex),
		szSize(_Move.szSize)
	{
		_Move.pBase = NULL;
		_Move.szIndex = 0;
		_Move.szSize = 0;
	}

	MemoryManager &operator=(MemoryManager &&_Move)
	{
		Clean();//先清理，防止泄露

		pBase = _Move.pBase;
		szIndex = _Move.szIndex;
		szSize = _Move.szSize;

		_Move.pBase = NULL;
		_Move.szIndex = 0;
		_Move.szSize = 0;

	}

	MemoryManager(size_t _szSize = 8) : pBase(NULL), szIndex(0), szSize(_szSize == 0 ? 1 : _szSize)//szSize保证至少为1
	{
		MemAlloc();
	}

	~MemoryManager(void)//析构
	{
		Clean();
	}

	void Reset(void)
	{
		szIndex = 0;//指针回到开头
		memset(pBase, 0, szSize);//置零
	}

	MemoryManager &operator+=(size_t szMoveSize)
	{
		CheckExpand(szMoveSize);//自动扩容
		szIndex += szMoveSize;
		return *this;
	}

	MemoryManager &operator-=(size_t szMoveSize)
	{
		if (szMoveSize <= szIndex)//防止向前越界
		{
			szIndex -= szMoveSize;
		}
		return *this;
	}

	MemoryManager &operator++(void)//前缀++
	{
		return operator+=(1);
	}

	MemoryManager &operator--(void)//前缀--
	{
		return operator-=(1);
	}

	uint8_t &operator*(void)//解引用
	{
		return pBase[szIndex];//szIndex在其他地方保证不越界，无需检查
	}

	uint8_t &operator[](size_t szIndexNoCheck)//危险
	{
		return pBase[szIndexNoCheck];
	}

	operator bool(void)
	{
		return pBase != NULL;
	}

	size_t GetIndex(void)//获取当前内存索引
	{
		return szIndex;
	}

	void SetIndex(size_t szIndexNoCheck)//危险
	{
		szIndex = szIndexNoCheck;
	}
};