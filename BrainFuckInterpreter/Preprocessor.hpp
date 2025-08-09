#pragma once

#include "CodeUnit.hpp"
#include "FileStream.hpp"
#include "CharStream.hpp"

#include <cstdio>
#include <vector>
#include <cctype>

template<typename StreamType>
class Preprocessor//Ԥ������
{
private:
	//ע��szDupCount��Ҫ���г�ʼ��������������������
	static size_t FindDuplicate(StreamType &sStream, const char cFind)
	{
		size_t szDupCount = 0;
		while(true)
		{
			char cRead = sStream.GetChar();
			if (sStream.Eof())
			{
				break;//�ļ��������뿪
			}

			//�ж��Ƿ���ͬ�������ظ��ַ���
			if (cRead != cFind)
			{
				sStream.UnGet();//����1��ȡ
				break;//������ͬ��ֱ���뿪
			}
			
			++szDupCount;//��ͬ�����ϲ�
		}

		return szDupCount;
	}
public:
	//����ú�������false����ô�κ�ʹ��listCode����ִ�еĲ�������δ������Ϊ����Ȼ����ȡʧ���ֳ���û�����
	static bool PreprocessInFile(StreamType &sStream, CodeList &listCode, bool bIgnoreUnknownChar = false)
	{
		if (!sStream)//�ļ���NULL������
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
			char cRead = sStream.GetChar();
			if (sStream.Eof())
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
					size_t szDupCount = FindDuplicate(sStream, cRead);
					CurCode.szMovOffset += szDupCount;
					szColumn += szDupCount;//������ȡ�����к�
				}
				break;
			case '<':
				{
					CurCode.enSymbol = CodeUnit::PrevMov;
					CurCode.szMovOffset = 1;
					//�����ظ�ֵ
					size_t szDupCount = FindDuplicate(sStream, cRead);
					CurCode.szMovOffset += szDupCount;
					szColumn += szDupCount;//������ȡ�����к�
				}
				break;
			case '+':
				{
					CurCode.enSymbol = CodeUnit::AddCur;
					CurCode.szCalcValue = 1;
					//�����ظ�ֵ
					size_t szDupCount = FindDuplicate(sStream, cRead);
					CurCode.szCalcValue += szDupCount;
					szColumn += szDupCount;//������ȡ�����к�
				}
				break;
			case '-':
				{
					CurCode.enSymbol = CodeUnit::SubCur;
					CurCode.szCalcValue = 1;
					//�����ظ�ֵ
					size_t szDupCount = FindDuplicate(sStream, cRead);
					CurCode.szCalcValue += szDupCount;
					szColumn += szDupCount;//������ȡ�����к�
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
					size_t szDupCount = FindDuplicate(sStream, cRead);

					//�ڴ�֮ǰ�����ж�һ��szStackTop������szDupCount������˵������δ��ԣ�����
					if (szDupCount > szStackTop)
					{
						printf("����ʧ��[line:%zu,column:%zu]������δƥ��\n", szLine, szColumn + szStackTop + 1);//ͨ��ʣ��δƥ����������λ�����
						return false;
					}

					szColumn += szDupCount;//�����к�

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
						char cRead = sStream.GetChar();
						if (sStream.Eof())
						{
							break;//�뿪ѭ�����ص�����жϣ����˳�
						}

						if (cRead == '\r' || cRead == '\n')
						{
							sStream.UnGet();//����1�ֽڣ��˳�ѭ��
							break;
						}
						//��������ȡ���ַ���ע�����豣����
					}
				}
				continue;//ע��˴�Ϊcontinue����break����������Ĭ��ѹ��
			default:
				{
					//�����У�ֻ���ڣ�\r��\n��\r\n����������������
					bool bNewLine = false;
					if (cRead == '\r')//�ж���һ���ǲ���\n
					{
						bNewLine = true;
						cRead = sStream.GetChar();
						if (!sStream.Eof() && cRead != '\n')
						{
							sStream.UnGet();//����1�ֽ�
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

					if (bIgnoreUnknownChar)
					{
						continue;//ֱ��ȫ�����������жϿհ׻���δ֪�ַ���
					}

					//���ǻ��У���������հ�
					if (isspace(cRead))
					{
						continue;//�����հף�ֱ�Ӽ���
					}

					//�����ǣ���ôδ֪�ַ��Ҳ���ע���ڣ�����
					printf("����ʧ��[line:%zu,column:%zu]������δ֪�ַ�\'%c\'\n", szLine, szColumn, cRead);
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