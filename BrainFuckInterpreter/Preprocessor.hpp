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

	static bool NoOptimizationPreprocess(StreamType &sStream, CodeList &listCode, bool bIgnoreUnknownChar)
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


	//�ж��Ƿ�ӵ���㹻��Ĵ����������Ż�������2����ΪʲôҪ+1����Ϊ�н�β�ڱ����ProgEndռ��1�ռ�
	static bool ContainEnoughCode(CodeList &listCode)
	{
		return listCode.size() < 2 + 1;
	}


	static bool IsOperator(CodeUnit::Symbol enSym)//�ж��Ƿ�����ֵ��������
	{
		return enSym == CodeUnit::Symbol::AddCur || enSym == CodeUnit::Symbol::SubCur;
	}

	static bool IsPointerMove(CodeUnit::Symbol enSym)//�ж��Ƿ���ָ����������
	{
		return enSym == CodeUnit::Symbol::NextMov || enSym == CodeUnit::Symbol::PrevMov;
	}

	static bool IsDuplicateRemoval(CodeUnit::Symbol enSym)//�ж��Ƿ�����Ҫȥ�صĴ�������
	{
		return enSym == CodeUnit::Symbol::ZeroMem;
	}

	static bool CanMergeOperator(CodeUnit::Symbol enTargetiSym, CodeUnit::Symbol enSourceSym)
	{
		return	IsOperator(enTargetiSym) && IsOperator(enSourceSym);//���������
	}
	static bool CanMergePointerMove(CodeUnit::Symbol enTargetiSym, CodeUnit::Symbol enSourceSym)
	{
		return IsPointerMove(enTargetiSym) && IsPointerMove(enSourceSym);//����ָ�����
	}
	static bool HasDuplicates(CodeUnit::Symbol enTargetiSym, CodeUnit::Symbol enSourceSym)
	{
		return IsDuplicateRemoval(enTargetiSym) && enTargetiSym == enSourceSym;//������Ҫȥ�ص����Ͳ������
	}


	//����ֵ�����Ƿ���Ҫ������ֵ������պ��෴�ϲ���Ϊ0���򷵻�false��ʾ���Զ����ϲ���
	template<typename T>
	static bool Merge(CodeUnit::Symbol &enSymLeft, T &tMergeLeft, CodeUnit::Symbol &enSymRight, T &tMergeRight)
	{
		if (enSymLeft == enSymRight)//������ͬ��ֱ�����
		{
			tMergeLeft += tMergeRight;
			return true;
		}

		//���Ų�ͬ���ȴ�С
		if (tMergeLeft > tMergeRight)//��ǰ����
		{
			tMergeLeft -= tMergeRight;//��ǰ���ֱ���������Ҫע����������ѧ��֤��
		}
		else if (tMergeLeft < tMergeRight)//��ǰ��С����Ҫ��ת��
		{
			tMergeLeft = tMergeRight - tMergeLeft;
			enSymLeft = enSymRight;//�ı����
		}
		else//�������ţ�ֱ������
		{
			return false;
		}

		return true;
	}



	/*
	�ϲ��Ż�ԭ���ϲ����ͬ��������Ķ����ͬ����
	��Դ�����ϲ��������ÿ����������������������뵥Ԫ
	������ÿ�γ��Ժϲ��������ϵ�ͬ�൥Ԫ���ɹ�����������
	���������������������������ϵĵ�Ԫ��������λ�ã�
	��������֮���������Ǳ��Ż������������������������󸲸ǣ�
	ֱ���������������뵥Ԫ�б�Ľ�β������ֱ����������λ�ã��Ż�����
	*/
	static void OperatorMergeOptimization(CodeList &listCode)//�����Ƿ����������1���Ż�
	{
		if (!ContainEnoughCode(listCode))//Ԫ�ز��㣬�����Ż���ֱ���˳�
		{
			return;
		}

		//���˱�Ȼ������2Ԫ��
		size_t szLast = 0;
		size_t szSize = listCode.size();
		for (size_t szCurrent = 1; szCurrent < szSize; ++szCurrent)
		{
			//���Ժϲ�
			auto &cuLast = listCode[szLast];
			auto &cuCurrent = listCode[szCurrent];
			
			if (CanMergeOperator(cuLast.enSymbol, cuCurrent.enSymbol))//�ϲ������
			{
				if (Merge(cuLast.enSymbol, cuLast.u8CalcValue, cuCurrent.enSymbol, cuCurrent.u8CalcValue))
				{
					continue;//�����ϲ�
				}
				//�ϲ����������ⲿ����
			}
			else if (CanMergePointerMove(cuLast.enSymbol, cuCurrent.enSymbol))//�ϲ�ָ���ƶ�
			{
				if (Merge(cuLast.enSymbol, cuLast.szMovOffset, cuCurrent.enSymbol, cuCurrent.szMovOffset))
				{
					continue;//�����ϲ�
				}
				//�ϲ����������ⲿ����
			}
			else if (HasDuplicates(cuLast.enSymbol, cuCurrent.enSymbol))//ȥ�ز���
			{
				continue;//ֱ�Ӽ���ֱ������һ����ƥ��
			}
			else//�޷��ϲ�
			{
				//�ƶ���������һ��
				listCode[++szLast] = std::move(listCode[szCurrent]);
				continue;//ע�����������ǰcontinue������Ҫ������ĺϲ���������
			}

			//�ϲ����������Ҫ����ֵ
			if (szLast > 0)
			{
				--szLast;//���������ƶ�����һ����Ԫ������У���������һ����������Ԫ���ܳ�������һ���ϲ�
			}
			else//��ͷ�ˣ��ģ�ֻ�ܳ��Ե������������ƶ�����ǰ��0�������
			{
				//ȷ��һ���Ƿ��ж��������ƶ�
				//��������Ϊĩβ���ڱ�����̫���ܻ�����������������Է���һ���԰�
				MyAssert(szCurrent + 1 < szSize, "�Ż������ڽ������ǰ����ĩβ��");

				//�ƶ���ǰ�ĵ�ͷ��������ִ���Ż�
				listCode[szLast] = std::move(listCode[++szCurrent]);
			}
		}
	}



	/*
	ѭ��������0�Ż�����ƥ��[-]��[+]��ʽ���滻ΪZ��
	����ҲҪƥ��[Z]����ֹ[[-]]֮���Ƕ�������Ҫ�ܶ���Ż�
	Ȼ���ڴ˻����ϵ���OperatorMergeOptimizationȥ���ظ���ZZZZ�Լ��Ż�����Ժϲ����µĵ�
	*/
	static void CountdownZeroOptimization(CodeList &listCode)
	{
		if (!ContainEnoughCode(listCode))//Ԫ�ز��㣬�����Ż���ֱ���˳�
		{
			return;
		}


	}




	/*
	ͬβѭ���Ż���
	ǰ���Ż���ɺ���������ѭ������ת�㣬ͬʱ�Ż�ͬβѭ����ͷ��ת��Ϊ���һ����ѭ����β��
	
	*/

	static void SameTailLoopOptimization(CodeList &listCode)
	{
		if (!ContainEnoughCode(listCode))//Ԫ�ز��㣬�����Ż���ֱ���˳�
		{
			return;
		}


	}



	static void Optimization(CodeList &listCode)
	{
		CountdownZeroOptimization(listCode);//���Ż�������ƥ��Ĺ̶�ģʽ
		OperatorMergeOptimization(listCode);//Ȼ����в���ȥ���Ż�
		SameTailLoopOptimization(listCode);//������ѭ���Ż���ͬʱ����ѭ����ת��ϵ
	}


public:

	//����ú�������false����ô�κ�ʹ��listCode����ִ�еĲ�������δ������Ϊ����Ȼ����ȡʧ���ֳ���û�����
	static bool PreprocessInStream(StreamType &sStream, CodeList &listCode, bool bIgnoreUnknownChar, bool bOptimization = true)
	{
		//�Ƚ������Ż�Ԥ�������Է����ã�
		if (!NoOptimizationPreprocess(sStream, listCode, bIgnoreUnknownChar))
		{
			return false;
		}

		//����������Ż��������Ż�����
		if (bOptimization)
		{
			//ע�⣺�κ���ʽ���Ż����ᵼ��ѭ����ת����ʧЧ������Optimization�����������
			Optimization(listCode);
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
	����+++��Ϊ+3��ͬʱ+3 -1���+2��>>>���>3��ͬʱ>3 <3ֱ�ӵ���
- �ڶ����Ż�����������ѭ����β��ȥ����������[-]��[+]�Ĺ����������Ϊֱ�Ӳ���
	�˲�������ܵ����µĺϲ�����Ҫ�ظ���һ���Ż��ͱ����Ż�

*/