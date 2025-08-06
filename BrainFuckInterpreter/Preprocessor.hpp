#pragma once

#include "CodeUnit.hpp"
#include "FileStream.hpp"

#include <cstdio>
#include <vector>


class Preprocessor//Ԥ������
{
private:
	CodeList listCode{};

	//ע��szDupCount��Ҫ���г�ʼ��������������������
	bool FindDuplicate(FileStream &sFile, const char cFind, size_t& szDupCount)
	{
		while(true)
		{
			char cRead;
			sFile.Read(cRead);
			if (sFile.Eof())
			{
				return false;//eof��������
			}

			//�ж��Ƿ���ͬ�������ظ��ַ���
			if (cRead != cFind)
			{
				sFile.MovFilePos(-1);//����1��ȡ
				return true;//������ͬ��ֱ�ӷ���
			}
			
			++szDupCount;//��ͬ�����ϲ�
		}
	}

public:
	bool PreprocessInFile(FileStream &sFile)
	{
		if (!sFile)
		{
			return false;
		}

		//�����ջ���洢������
		std::vector<size_t> codeBlockStack;

		//��ȡ��ת���������б�
		while (true)
		{
			char cRead;
			sFile.Read(cRead);
			if (sFile.Eof())
			{
				break;//�����뿪
			}

			CodeUnit CurCode{};
			switch (cRead)
			{
			case '>':
				{
					CurCode.enSymbol = CodeUnit::NextMov;
					CurCode.szMovOffset = 1;
					//�����ظ�ֵ
					if (!FindDuplicate(sFile, cRead, CurCode.szMovOffset))
					{
						printf("����ʧ�ܣ���ǰ�����ļ�β\n");
						return false;
					}
				}
				break;
			case '<':
				{
					CurCode.enSymbol = CodeUnit::PrevMov;
					CurCode.szMovOffset = 1;
					//�����ظ�ֵ
					if (!FindDuplicate(sFile, cRead, CurCode.szMovOffset))
					{
						printf("����ʧ�ܣ���ǰ�����ļ�β\n");
						return false;
					}
				}
				break;
			case '+':
				{
					CurCode.enSymbol = CodeUnit::AddCur;
					CurCode.szCalcValue = 1;
					//�����ظ�ֵ
					if (!FindDuplicate(sFile, cRead, CurCode.szCalcValue))
					{
						printf("����ʧ�ܣ���ǰ�����ļ�β\n");
						return false;
					}
				}
				break;
			case '-':
				{
					CurCode.enSymbol = CodeUnit::SubCur;
					CurCode.szCalcValue = 1;
					//�����ظ�ֵ
					if (!FindDuplicate(sFile, cRead, CurCode.szCalcValue))
					{
						printf("����ʧ�ܣ���ǰ�����ļ�β\n");
						return false;
					}
				}
				break;
			case '.':
				{
					CurCode.enSymbol = CodeUnit::OptCur;
				}
				break;
			case ',':
				{
					CurCode.enSymbol = CodeUnit::IptCur;
				}
				break;
			case '['://��Ԥ������ѹ��ջ���ȴ�]������תƫ����
				{
					CurCode.enSymbol = CodeUnit::LoopBeg;
					listCode.push_back(CurCode);//��ǰѹ��
					codeBlockStack.push_back(listCode.size() - 1);//���������Ա�����޸�
				}
				continue;//ע��˴�Ϊcontinue����break����������Ĭ��ѹ��
			case ']':
				{
					if (codeBlockStack.empty())//ʲô������û�������ţ������������ţ�ȥ���
					{
						printf("����ʧ�ܣ�����δƥ��\n");
						return false;
					}

					//��ȡջ������
					size_t szStackTop = codeBlockStack.size() - 1;

					CurCode.enSymbol = CodeUnit::LoopEnd;//�ҵ�һ��loopend��Ѱ����������
					CurCode.szJmpIndex = codeBlockStack[szStackTop];//��ȡջ����Ҳ���������һ��loopbeg��������תλ�ã��õ�ǰ����ת��loopbeg
					listCode.push_back(CurCode);//��ǰ���룬��ʼ�����ظ�ѭ����

					//���ڣ���ȡ�����ظ���]����ǰ����֮��Ե�����loopbeg����ת���ظ�����󣬶���ÿ���ظ���]������ȷ���
					//�����������ظ�loopend��������ת�������жϿ���
					size_t szDupCount = 0;
					if (!FindDuplicate(sFile, cRead, szDupCount))
					{
						printf("����ʧ�ܣ���ǰ�����ļ�β\n");
						return false;
					}

					//�ڴ�֮ǰ�����ж�һ��szStackTop������szDupCount������˵������δ��ԣ�����
					if (szDupCount > szStackTop)
					{
						printf("����ʧ�ܣ�����δƥ��\n");
						return false;
					}

					//��ȡszDupCount����β��������ѭ���飬�������һ��ѭ������Ϊǰ�������жϵ���תĿ��
					while (szDupCount-- > 0)//ѭ��szDupCount��
					{
						CurCode.enSymbol = CodeUnit::LoopEnd;
						CurCode.szJmpIndex = codeBlockStack[--szStackTop];//ȡ������һ����loopbeg�������
						listCode.push_back(CurCode);//����
					}

					//�����loopend��תĿ��ȫ��ƥ�����
					//����loopbeg��δƥ�䣬��ȡ���һ��λ�ú�ѭ��д��
					size_t szJmpIndex = listCode.size();//��Ϊ����loopend�������ˣ���ǰβ��index�������һ��λ��

					//����ǰ�����е�loopbeg����תλ��Ϊ��λ��֮��
					for (size_t szStackIndex = szStackTop; szStackIndex != codeBlockStack.size(); ++szStackIndex)
					{
						size_t szListIndex = codeBlockStack[szStackIndex];//��ȡloopbeg��list�е�idx
						listCode[szListIndex].szJmpIndex = szJmpIndex;//����Ŀ��λ��
					}

					//���ˣ������ѳɣ�
					//��ջ����Ļ���ȫ�����������szStackTop��Ϊsize����index������Ԫ�ؼ���ΪszStackTop��
					codeBlockStack.resize(szStackTop);
				}
				continue;//ע��˴�Ϊcontinue����break����������Ĭ��ѹ��
			case '?':
				{
					CurCode.enSymbol = CodeUnit::DbgInfo;
				}
				break;
			case '#':
				{
					//����ע��
				}
				continue;//ע��˴�Ϊcontinue����break����������Ĭ��ѹ��
			default:
				//�����к��ַ���
				return false;//ע�ⱨ��ֱ�ӷ���
			}

			//ѹ���б�
			listCode.push_back(CurCode);
		}

		//�ж�ջ���Ƿ��в��������ţ��ǣ����������δ��ԣ�����
		if (!codeBlockStack.empty())
		{
			printf("����ʧ�ܣ�����δƥ��\n");
			return false;
		}
		
		return true;
	}










};