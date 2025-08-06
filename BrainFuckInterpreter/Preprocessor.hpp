#pragma once

#include "CodeUnit.hpp"
#include "FileStream.hpp"

#include <cstdio>
#include <vector>
#include <cctype>


class Preprocessor//Ԥ������
{
private:
	//ע��szDupCount��Ҫ���г�ʼ��������������������
	static void FindDuplicate(FileStream &sFile, const char cFind, size_t& szDupCount)
	{
		while(true)
		{
			char cRead;
			sFile.Read(cRead);
			if (sFile.Eof())
			{
				return;//�����ϼ�����
			}

			//�ж��Ƿ���ͬ�������ظ��ַ���
			if (cRead != cFind)
			{
				sFile.MovFilePos(-1);//����1��ȡ
				return;//������ͬ��ֱ�ӷ���
			}
			
			++szDupCount;//��ͬ�����ϲ�
		}
	}
public:
	//����ú�������false����ô�κ�ʹ��listCode����ִ�еĲ�������δ������Ϊ����Ȼ����ȡʧ���ֳ���û�����
	static bool PreprocessInFile(FileStream &sFile, CodeList &listCode)
	{
		if (!sFile)//�ļ���NULL������
		{
			return false;
		}

		listCode.clear();//�����Ա��ȡ

		//�����ջ���洢������
		std::vector<size_t> codeBlockStack;

		//������Ϣ
		size_t szLine = 1;
		size_t szColumn = 1;

		//��ȡ��ת���������б�
		while (true)
		{
			char cRead;
			sFile.Read(cRead);
			if (sFile.Eof())
			{
				break;//�����뿪
			}

			//ÿ��һ���ַ�����һ��
			++szColumn;//�����к�

			CodeUnit CurCode{};
			switch (cRead)
			{
			case '>':
				{
					CurCode.enSymbol = CodeUnit::NextMov;
					CurCode.szMovOffset = 1;
					//�����ظ�ֵ
					FindDuplicate(sFile, cRead, CurCode.szMovOffset);
				}
				break;
			case '<':
				{
					CurCode.enSymbol = CodeUnit::PrevMov;
					CurCode.szMovOffset = 1;
					//�����ظ�ֵ
					FindDuplicate(sFile, cRead, CurCode.szMovOffset);
				}
				break;
			case '+':
				{
					CurCode.enSymbol = CodeUnit::AddCur;
					CurCode.szCalcValue = 1;
					//�����ظ�ֵ
					FindDuplicate(sFile, cRead, CurCode.szCalcValue);
				}
				break;
			case '-':
				{
					CurCode.enSymbol = CodeUnit::SubCur;
					CurCode.szCalcValue = 1;
					//�����ظ�ֵ
					FindDuplicate(sFile, cRead, CurCode.szCalcValue);
				}
				break;
			case '.':
				{
					CurCode.enSymbol = CodeUnit::OptCur;//������ָ����ţ�������Ϣ��Ч
				}
				break;
			case ',':
				{
					CurCode.enSymbol = CodeUnit::IptCur;//������ָ����ţ�������Ϣ��Ч
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
						printf("����ʧ��[line:%zu,column:%zu]������δƥ��\n", szLine, szColumn);
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
					FindDuplicate(sFile, cRead, szDupCount);

					//�ڴ�֮ǰ�����ж�һ��szStackTop������szDupCount������˵������δ��ԣ�����
					if (szDupCount > szStackTop)
					{
						printf("����ʧ��[line:%zu,column:%zu]������δƥ��\n", szLine, szColumn);
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
					CurCode.enSymbol = CodeUnit::DbgInfo;//������ָ����ţ�������Ϣ��Ч
				}
				break;
			case '#':
				{
					//����ע��
					//��ȡ��ֱ�����з��е�����һ�������˲���default�����Ա�ͳһ�к��к�
					while (true)
					{
						char cRead;
						sFile.Read(cRead);
						if (sFile.Eof())
						{
							break;//�뿪ѭ�����ص�����жϣ����˳�
						}

						if (cRead == '\r' || cRead == '\n')
						{
							sFile.MovFilePos(-1);//����1�ֽڣ��˳�ѭ��
							break;
						}
						//��������ȡ���ַ���ע�����豣����
					}
				}
				continue;//ע��˴�Ϊcontinue����break����������Ĭ��ѹ��
			default:
				{
					//������
					bool bNewLine = false;
					if (cRead == '\r')//�ж���һ���ǲ���\n
					{
						bNewLine = true;
						sFile.Read(cRead);
						if (!sFile.Eof() && cRead != '\n')
						{
							sFile.MovFilePos(-1);//����1�ֽ�
						}//�˴�����eofҲ���õ��ģ����º�ͻ�continue��Ȼ���˳�ѭ��
					}
					else if (cRead == '\n')
					{
						bNewLine = true;
					}

					if (bNewLine)
					{
						++szLine;//�����к�
						szColumn = 1;//�����к�
						continue;//ֱ�Ӽ���
					}

					//���ǻ��У���������հ�
					if (isspace(cRead))
					{
						continue;//�����հף�ֱ�Ӽ���
					}

					//�����ǣ���ôδ֪�ַ��Ҳ���ע���ڣ�����
					printf("����ʧ��[line:%zu,column:%zu]������δ֪�ַ�:[%c]\n", szLine, szColumn, cRead);
				}
				return false;//ע�ⱨ��ֱ�ӷ���
			}

			//ѹ���б�
			listCode.push_back(CurCode);
		}

		//�ж�ջ���Ƿ��в��������ţ��ǣ����������δ��ԣ�����
		if (!codeBlockStack.empty())
		{
			printf("����ʧ��[line:%zu,column:%zu]������δƥ��\n", szLine, szColumn);
			return false;
		}

		//��ɣ�β������һ�������ַ���Ҳ����ProgEnd��ִ�е�����������
		listCode.push_back(CodeUnit{ .enSymbol = CodeUnit::ProgEnd });
		
		return true;
	}
	//Ԥ�����������д�����

};