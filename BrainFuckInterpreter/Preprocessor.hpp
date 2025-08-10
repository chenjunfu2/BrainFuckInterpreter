#pragma once

#include "CodeUnit.hpp"
#include "FileStream.hpp"
#include "StrStream.hpp"

#include <cstdio>
#include <vector>
#include <cctype>

template<typename StreamType>
class Preprocessor//Ԥ������
{
private:
	//static size_t FindDuplicate(StreamType &sStream, const char cFind)
	//{
	//	size_t szDupCount = 0;
	//	while(true)
	//	{
	//		char cRead = sStream.GetChar();
	//		if (sStream.Eof())
	//		{
	//			break;//�ļ��������뿪
	//		}
	//
	//		//�ж��Ƿ���ͬ�������ظ��ַ���
	//		if (cRead != cFind)
	//		{
	//			sStream.UnGet();//����1��ȡ
	//			break;//������ͬ��ֱ���뿪
	//		}
	//		
	//		++szDupCount;//��ͬ�����ϲ�
	//	}
	//
	//	return szDupCount;
	//}

	static bool ZeroPreprocess(StreamType &sStream, CodeList &listCode, bool bIgnoreUnknownChar = false)
	{
		if (!sStream)//�ļ���NULL������
		{
			return false;
		}

		//�����ջ���洢������
		std::vector<size_t> codeBlockStack;

		listCode.clear();//�����Ա��ȡ

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

			//��0��Ԥ���������κ��Ż�������debug�뱨�����
			CodeUnit CurCode{};
			switch (cRead)
			{
			case '>':
				{
					CurCode.enSymbol = CodeUnit::NextMov;
					CurCode.szMovOffset = 1;
				}
				break;
			case '<':
				{
					CurCode.enSymbol = CodeUnit::PrevMov;
					CurCode.szMovOffset = 1;
				}
				break;
			case '+':
				{
					CurCode.enSymbol = CodeUnit::AddCur;
					CurCode.u8CalcValue = 1;
				}
				break;
			case '-':
				{
					CurCode.enSymbol = CodeUnit::SubCur;
					CurCode.u8CalcValue = 1;
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

					//�ҵ�һ��loopend��Ѱ���������ԣ�
					//��ȡջ����Ҳ�������һ���뵱ǰƥ��loopbeg��Ȼ�󵯳�
					size_t listCodeIndex = codeBlockStack.back();
					codeBlockStack.pop_back();

					CurCode.enSymbol = CodeUnit::LoopEnd;
					CurCode.szJmpIndex = listCodeIndex;//������תλ�ã��õ�ǰ����ת��loopbeg
					listCode.push_back(CurCode);//��ǰ���룬�Ա����size

					//����loopbeg��תĿ��λ��Ϊ��ǰ֮�����������������������ѭ������Ϊ��ǰ�Ѿ����룬����size�պ���β��index��
					listCode[listCodeIndex].szJmpIndex = listCode.size();
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
		//ͬʱ��ȷ��ѭ����ת��ĩβ����ȷ���������index out of range
		listCode.push_back(CodeUnit{ .enSymbol = CodeUnit::ProgEnd });

		return true;
	}


	static bool IsOperator(CodeUnit::Symbol enSym)
	{
		return enSym == CodeUnit::Symbol::AddCur || enSym == CodeUnit::Symbol::SubCur;
	}

	static bool IsPointerMove(CodeUnit::Symbol enSym)
	{
		return enSym == CodeUnit::Symbol::NextMov || enSym == CodeUnit::Symbol::PrevMov;
	}

	static bool CanMergeOperator(CodeUnit::Symbol enTargetiSym, CodeUnit::Symbol enSourceSym)
	{
		return	IsOperator(enTargetiSym) && IsOperator(enSourceSym);//���������
	}
	static bool CanMergePointerMove(CodeUnit::Symbol enTargetiSym, CodeUnit::Symbol enSourceSym)
	{
		return IsPointerMove(enTargetiSym) && IsPointerMove(enSourceSym);//����ָ�����
	}


	static bool FirstOptimization(CodeList &listCode)//�����Ƿ����������1���Ż�
	{
		bool bIsOpti = false;

		CodeList listOptiCode{};
		bool bPushNew = true;//��һ����������һ��Ԫ�ط�ֹlistOptiCode.backը��
		for (auto &it : listCode)//��ҩ��˵ʲôsizeѭ�������п����ˣ����������Ż��İ����
		{
			if (bPushNew)
			{
				listOptiCode.push_back(it);//����һ���µĲ���������һ��
				bPushNew = false;
				continue;
			}

			//���Ժϲ�
			auto &cuLast = listOptiCode.back();
			
			if (CanMergeOperator(cuLast.enSymbol, it.enSymbol))//�ϲ������
			{
				bIsOpti = true;
				if (cuLast.enSymbol != it.enSymbol)//���Ų�ͬ���ȴ�С
				{
					if (cuLast.u8CalcValue > it.u8CalcValue)//��ǰ����
					{
						cuLast.u8CalcValue -= it.u8CalcValue;//��ǰ���ֱ���������Ҫע����������ѧ��֤��
					}
					else if(cuLast.u8CalcValue < it.u8CalcValue)//��ǰ��С����Ҫ��ת��
					{
						cuLast.u8CalcValue = it.u8CalcValue - cuLast.u8CalcValue;
						cuLast.enSymbol = it.enSymbol;
					}
					else//�������ţ�ֱ������
					{
						listOptiCode.pop_back();//ɾ��ĩβ�Ҳ����뵱ǰֵ
						bPushNew = true;
					}
				}
				else//������ͬ��ֱ�����
				{
					cuLast.u8CalcValue += it.u8CalcValue;
				}
			}
			else if (CanMergePointerMove(cuLast.enSymbol, it.enSymbol))//�ϲ�ָ���ƶ�
			{
				bIsOpti = true;
				if (cuLast.enSymbol != it.enSymbol)//���Ų�ͬ���ȴ�С
				{
					if (cuLast.szMovOffset > it.szMovOffset)//��ǰ����
					{
						cuLast.szMovOffset -= it.szMovOffset;//��ǰ���ֱ���������Ҫע����������ѧ��֤��
					}
					else if (cuLast.szMovOffset < it.szMovOffset)//��ǰ��С����Ҫ��ת��
					{
						cuLast.szMovOffset = it.szMovOffset - cuLast.szMovOffset;
						cuLast.enSymbol = it.enSymbol;
					}
					else//�������ţ�ֱ������
					{
						listOptiCode.pop_back();//ɾ��ĩβ�Ҳ����뵱ǰֵ
						bPushNew = true;
					}
				}
				else//������ͬ��ֱ�����
				{
					cuLast.szMovOffset += it.szMovOffset;
				}
			}
			else//�޷��ϲ�
			{
				listOptiCode.push_back(it);//ֱ�Ӳ��룬ע�ⲻҪ���ñ�־λ���Ա���һ��ѭ���ж�
			}
		}

		//���
		if (bIsOpti)//���ٽ���һ���Ż����⸲��ԭ�ȵĴ���
		{
			listCode = std::move(listOptiCode);
		}//���û���κ��Ż���ֱ���Զ�����

		return bIsOpti;
	}

	static bool SecondOptimization(CodeList &listCode)//�����Ƿ����������1���Ż�
	{





	}





	static bool FirstPreprocess(CodeList &listCode)//�����Ƿ�ɹ�
	{

	}

	static bool SecondPreprocess(CodeList &listCode)//�����Ƿ�ɹ�
	{

	}



public:
	enum PreprocessLevel :uint8_t
	{
		Level_Min = 0,
		Level_Zero = Level_Min,
		Level_First,
		Level_Second,
		Level_Max = Level_Second,
	};

	//����ú�������false����ô�κ�ʹ��listCode����ִ�еĲ�������δ������Ϊ����Ȼ����ȡʧ���ֳ���û�����
	static bool PreprocessInStream(StreamType &sStream, CodeList &listCode, bool bIgnoreUnknownChar = false, PreprocessLevel enLevel = Level_Max)
	{
		if (enLevel >= Level_Zero)
		{
			if (!ZeroPreprocess(sStream, listCode, bIgnoreUnknownChar))
			{
				return false;
			}
		}

		if (enLevel >= Level_First)
		{
			if (!FirstPreprocess(listCode))
			{
				return false;
			}

		}

		if (enLevel >= Level_Second)
		{
			if (!SecondPreprocess(listCode))
			{
				return false;
			}
		}

		return true;
	}
	//Ԥ�����������д�����

	//Ԥ�������Ż��д����ƣ�����ʶ�𾭵��0�������ƶ��������ݼ�����������Ƕ�׵����ȣ�

};


/*
��Ϊ��ν�����
- �����Ԥ�������Ż����㱨��
- ��һ��Ԥ����ѭ�����õ�һ���Ż�
- �ڶ���Ԥ�������õڶ����Ż���ѭ������1~2ֱ��ȫ�����


- ��һ���Ż����ϲ������������������������ָ�������
	����+++��Ϊ+3��ͬʱ+3 -1���+2��>>>���>3��ͬʱ>3 <3ֱ�ӵ�����
	��Ϊ���ڵ������ºϲ���������Լ����Ż��������������Ҫѭ��ֱ���޷��Ż�Ϊֹ
- �ڶ����Ż�����������ѭ����β��ȥ����������[-]��[+]�Ĺ����������Ϊֱ�Ӳ���
	�˲�������ܵ����µĺϲ�����Ҫ�ظ���һ���Ż��ͱ����Ż�

*/