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
	static bool ContainsEnoughCode(CodeList &listCode)
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
	static bool IsZeroMem(CodeUnit::Symbol enSym)
	{
		return enSym == CodeUnit::Symbol::ZeroMem;
	}
	static bool IsDuplicateRemoval(CodeUnit::Symbol enSym)//�ж��Ƿ�����Ҫȥ�صĴ�������
	{
		return IsZeroMem(enSym);
	}

	static bool CanChangeToZeroMem(CodeUnit::Symbol enSym)//�ж��Ƿ�����Ҫ�Ż�Ϊ�ڴ���0����
	{
		return IsOperator(enSym) || IsZeroMem(enSym);
	}


	/*
	ѭ��������0�Ż�����ƥ��[-]��[+]��ʽ���滻ΪZ��
	����ҲҪƥ��[Z]����ֹ[[-]]֮���Ƕ�������Ҫ�ܶ���Ż�
	Ȼ���ڴ˻����ϵ���OperatorMergeOptimizationȥ���ظ���ZZZZ�Լ��Ż�����Ժϲ����µĵ�
	*/
	static void CountdownZeroOptimization(CodeList &listCode)
	{
		if (!ContainsEnoughCode(listCode))//Ԫ�ز��㣬�����Ż���ֱ���˳�
		{
			return;
		}

		//�����ջ���洢������
		std::vector<size_t> codeBlockStack;

		size_t szLast = 0;//��Ȼ�ǿ���ָ�����
		for (size_t szCurrent = 0, szCodeSize = listCode.size(); szCurrent < szCodeSize; ++szCurrent, ++szLast)
		{
			if (szLast != szCurrent)
			{
				listCode[szLast] = std::move(listCode[szCurrent]);
			}

			if (listCode[szLast].enSymbol == CodeUnit::Symbol::LoopBeg)
			{
				codeBlockStack.push_back(szLast);
				continue;
			}

			if (listCode[szLast].enSymbol != CodeUnit::Symbol::LoopEnd)
			{
				continue;
			}

			//����������һ��LoopEnd
			//�鿴codeBlockStackջ���������֮ƥ���LoopBeg�����뵱ǰLoopEnd�Ĳ�ֵ
			//�����ֵ�պ�Ϊ2��������ƥ�䵽[x]���У��ж�x�����ͣ�Ȼ�����
			if (szLast - codeBlockStack.back() != 2)
			{
				//������2����ô����������
				codeBlockStack.pop_back();
				continue;
			}

			//�ܺã�����ƥ��һ���ڲ��ķ�������
			if (CanChangeToZeroMem(listCode[szLast - 1].enSymbol) &&
				listCode[szLast - 1].u8CalcValue == 1)//���뱣֤u8CalcValue��1��������ܵ���[++]֮��Ĳ���������Ԥ��
			{
				//�ǳ������������Ҫ�Ż��Ĳ���
				size_t szNewLast = codeBlockStack.back();
				codeBlockStack.pop_back();
				//�ڴ沼�־������£�
				//>  [  +  ]  x  x  <
				//3  4  5  6  7  8  9
				//   ^     ^     ^
				//  top   lst   cur 

				//����szNewLast����top
				//��szNewLast��Symbol��ΪZeroMem
				listCode[szNewLast].enSymbol = CodeUnit::Symbol::ZeroMem;
				listCode[szNewLast].szMovOffset = 0;//���������ڴ�
				//��szLast��ΪNewLast��������ָ��Ϊ�Ż�λ��
				szLast = szNewLast;

				//�ڴ沼���������£�
				//>  Z  x  x  x  x  <
				//3  4  5  6  7  8  9
				//   ^           ^
				//  lst         cur 
				//for����һ�ε�����ʱ���ִ�в����ƶ�����һλ������
				continue;
			}
		}
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
	static bool MergeOperator(CodeUnit::Symbol &enSymLeft, T &tMergeLeft, CodeUnit::Symbol &enSymRight, T &tMergeRight)
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
		if (!ContainsEnoughCode(listCode))//Ԫ�ز��㣬�����Ż���ֱ���˳�
		{
			return;
		}

		//���˱�Ȼ������2Ԫ��
		size_t szLast = 0;
		for (size_t szCurrent = 1, szCodeSize = listCode.size(); szCurrent < szCodeSize; ++szCurrent)
		{
			//���Ժϲ�
			auto &cuLast = listCode[szLast];
			auto &cuCurrent = listCode[szCurrent];
			
			if (CanMergeOperator(cuLast.enSymbol, cuCurrent.enSymbol))//�ϲ������
			{
				if (MergeOperator(cuLast.enSymbol, cuLast.u8CalcValue, cuCurrent.enSymbol, cuCurrent.u8CalcValue))
				{
					continue;//�����ϲ�
				}
				//�ϲ����������ⲿ����
			}
			else if (CanMergePointerMove(cuLast.enSymbol, cuCurrent.enSymbol))//�ϲ�ָ���ƶ�
			{
				if (MergeOperator(cuLast.enSymbol, cuLast.szMovOffset, cuCurrent.enSymbol, cuCurrent.szMovOffset))
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
				MyAssert(szCurrent + 1 < szCodeSize, "�Ż������ڽ������ǰ����ĩβ��");

				//�ƶ���ǰ�ĵ�ͷ��������ִ���Ż�
				listCode[szLast] = std::move(listCode[++szCurrent]);
			}
		}

		//��ɣ��ü����뵽szLast + 1��λ�ã�ע�⣬����������ڱ�Ҳ�����ڲ�����Ȼ���ƶ��������ٴδ���
		listCode.resize(szLast + 1);
	}



	/*
	��Чѭ���Ż����жϵ���0���ѭ����ɾ����������û�õ�
	�������Ƕ��ѭ��ɾ����[[[[[++>++<<--]]]]]
	ɾ��������жϣ�����ɾ������������ѭ��

	�������ѶȽϴ���Ҫ����ģʽƥ��
	*/
	static void InvalidLoopOptimization(CodeList &listCode)
	{
		if (!ContainsEnoughCode(listCode))//Ԫ�ز��㣬�����Ż���ֱ���˳�
		{
			return;
		}

		size_t szLast = 0;//��Ȼ�ǿ���ָ�����
		for (size_t szCurrent = 0, szCodeSize = listCode.size(); szCurrent < szCodeSize; ++szCurrent, ++szLast)
		{
			if (szLast != szCurrent)
			{
				listCode[szLast] = std::move(listCode[szCurrent]);
			}






		}





	}


	/*
	ѭ����ת�������Ż���
	ǰ���Ż���ɺ���������ѭ������ת�㣬ͬʱ�Ż�ͬβѭ����ͷ��ת��Ϊ���һ����ѭ����β��
	*/

	static void LoopSettingAndOptimization(CodeList &listCode)
	{
		if (!ContainsEnoughCode(listCode))//Ԫ�ز��㣬�����Ż���ֱ���˳�
		{
			return;
		}


	}



	static void Optimization(CodeList &listCode)
	{
		//��Ҫ�޸�Ϊ�����Ƿ����������һ���Ż�
		//Ȼ��˳�����У�ֱ��ĳһ�����еĺ���
		//������false������û�и�������Ż�����

		CountdownZeroOptimization(listCode);//���Ż�������ƥ��Ĺ̶�ģʽ
		
		OperatorMergeOptimization(listCode);//���Ž��в���ȥ���Ż�
		InvalidLoopOptimization(listCode);//Ȼ���Ż�����Чѭ��

		LoopSettingAndOptimization(listCode);//������ѭ���Ż���ͬʱ����ѭ����ת��ϵ
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

	static void PrintCodeList(const CodeList &listCode)
	{
		for (const auto &it : listCode)
		{
			switch (it.enSymbol)
			{
			case CodeUnit::NextMov:
			case CodeUnit::PrevMov:
				{
					printf("%s%zu ", CodeUnit::BfCodeChar[it.enSymbol], it.szMovOffset);
				}
				break;
			case CodeUnit::AddCur:
			case CodeUnit::SubCur:
				{
					printf("%s%zu ", CodeUnit::BfCodeChar[it.enSymbol], it.u8CalcValue);
				}
				break;
			case CodeUnit::ProgEnd:
			case CodeUnit::OptCur:
			case CodeUnit::IptCur:
			case CodeUnit::LoopBeg:
			case CodeUnit::LoopEnd:
			case CodeUnit::DbgInfo:
			case CodeUnit::ZeroMem:
				{
					printf("%s ", CodeUnit::BfCodeChar[it.enSymbol]);
				}
				break;
			case CodeUnit::Unknown:
			default:
				break;
			}
		}
	}


	//Ԥ�����������д�����

	//Ԥ�������Ż��д����ƣ�����ʶ�𾭵��0�������ƶ��������ݼ�����������Ƕ�׵����ȣ�


	/*
	TODO:Ԥ������CodeUnit���浽�ļ�����׺.bfvc��brainfuck virtual code
	ִ������ʱ����ʶ���ļ����ͣ�bfvc�����ļ�ͷ��BFVC��ͷ������ļ���Ϣ��
		����.bfԴ���룬��������ת�������ݾ�����Ҫ�����Ż�ִ��
		.bfvc�����У�Ҳ���ܽ����κ��Ż�������Ҫ��������ļ��Ϸ��ԣ�������Ե�
	*/

};