#pragma once

#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstdio>

class MemoryManager//�ڴ������
{
private:
	uint8_t *pBase = NULL;//����ַָ��
	size_t szIndex = 0;//��ǰλ���±�
	size_t szSize = 0;//�����ܴ�С
private:
	constexpr const static size_t DEFAULT_EXPAND_FACTOR = 2;//Ĭ�϶���

	void Clean(void)//����ע���ɾ���ڴ棡
	{
		free(pBase);
		pBase = NULL;
		szIndex = 0;
		szSize = 0;
	}

	bool IsMulOverflow(size_t szLeft, size_t szRight)//szRightд���ٸı�ı����������ܸ���
	{
		return szLeft > (SIZE_MAX / szRight);
	}

	void MemAlloc(void)//�����ã���������λ��ʹ��
	{
		pBase = (uint8_t *)malloc(szSize);
		if (pBase == NULL)
		{
			Clean();
			printf("�ڴ����ʧ��");
			exit(0);
		}
		memset(pBase, 0, szSize);//����
	}

	void CheckExpand(size_t szMoveSize)
	{
		if (szIndex + szMoveSize >= szSize)
		{
			Expand(szIndex + szMoveSize);
		}
	}

	void Expand(size_t szNewSize)//����
	{
		if (szNewSize <= szSize)//�ȵ�ǰ��С�����ݸ�p
		{
			return;
		}

		//����szNewSize�ȵ�ǰszSize����1��2������
		size_t szExpandFactor = DEFAULT_EXPAND_FACTOR;
		while (szSize * szExpandFactor < szNewSize)
		{
			//����ǰ���
			if (IsMulOverflow(szExpandFactor, DEFAULT_EXPAND_FACTOR))
			{
				Clean();
				printf("�ڴ�����ʧ�ܣ��������\n");
				exit(-1);
			}

			szExpandFactor *= DEFAULT_EXPAND_FACTOR;//����ֱ�����ڻ����

			//����ǰ���
			if (IsMulOverflow(szExpandFactor, szSize))
			{
				Clean();
				printf("�ڴ�����ʧ�ܣ��������\n");
				exit(-1);
			}
		}

		//�ط���
		uint8_t *pNewBase = (uint8_t *)realloc(pBase, szSize * szExpandFactor);
		if (pNewBase == NULL)
		{
			Clean();
			printf("�ڴ�����ʧ�ܣ��ڴ治��\n");
			exit(-1);
		}
		//���ݳɹ��滻ָ��
		pBase = pNewBase;
		//��λ�ã����Σ�����
		memset(&pBase[szSize], 0, szSize);
		szSize *= szExpandFactor;
	}
public:
	MemoryManager(const MemoryManager &) = delete;//��ֹ����
	MemoryManager &operator=(const MemoryManager &) = delete;//��ֹ��ֵ

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
		Clean();//��������ֹй¶

		pBase = _Move.pBase;
		szIndex = _Move.szIndex;
		szSize = _Move.szSize;

		_Move.pBase = NULL;
		_Move.szIndex = 0;
		_Move.szSize = 0;

	}

	MemoryManager(size_t _szSize = 8) : pBase(NULL), szIndex(0), szSize(_szSize == 0 ? 1 : _szSize)//szSize��֤����Ϊ1
	{
		MemAlloc();
	}

	~MemoryManager(void)//����
	{
		Clean();
	}

	void Reset(void)
	{
		szIndex = 0;//ָ��ص���ͷ
		memset(pBase, 0, szSize);//����
	}

	MemoryManager &operator+=(size_t szMoveSize)
	{
		CheckExpand(szMoveSize);//�Զ�����
		szIndex += szMoveSize;
		return *this;
	}

	MemoryManager &operator-=(size_t szMoveSize)
	{
		if (szMoveSize <= szIndex)//��ֹ��ǰԽ��
		{
			szIndex -= szMoveSize;
		}
		return *this;
	}

	MemoryManager &operator++(void)//ǰ׺++
	{
		return operator+=(1);
	}

	MemoryManager &operator--(void)//ǰ׺--
	{
		return operator-=(1);
	}

	uint8_t &operator*(void)//������
	{
		return pBase[szIndex];//szIndex�������ط���֤��Խ�磬������
	}

	uint8_t &operator[](size_t szIndexNoCheck)//Σ��
	{
		return pBase[szIndexNoCheck];
	}

	operator bool(void)
	{
		return pBase != NULL;
	}

	size_t GetIndex(void)//��ȡ��ǰ�ڴ�����
	{
		return szIndex;
	}

	void SetIndex(size_t szIndexNoCheck)//Σ��
	{
		szIndex = szIndexNoCheck;
	}
};