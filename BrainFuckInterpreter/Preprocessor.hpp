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
		return listCode.size() >= 2 + 1;
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
	static bool CountdownZeroOptimization(CodeList &listCode)
	{
		if (!ContainsEnoughCode(listCode))//Ԫ�ز��㣬�����Ż���ֱ���˳�
		{
			return false;
		}

		//�����ջ���洢������
		std::vector<size_t> codeBlockStack;

		bool bIsOptimization = false;

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

			MyAssert(!codeBlockStack.empty(), "�Ż�ʧ�ܣ������ջ����Ϊ�գ�");//��ô�������أ�����ǰ���Ѿ�ƥ��������أ�

			//�������Ѿ�����������loopbegƥ���loopend�����ڱ���ջ��ֵȻ�󵯳��Ա�ƽջ
			size_t szNewLast = codeBlockStack.back();
			codeBlockStack.pop_back();

			//����������һ��LoopEnd
			//�鿴codeBlockStackջ���������֮ƥ���LoopBeg�����뵱ǰLoopEnd�Ĳ�ֵ
			//�����ֵ�պ�Ϊ2��������ƥ�䵽[x]���У��ж�x�����ͣ�Ȼ�����
			if (szLast - szNewLast != 2)
			{
				//������2����ô������ѭ��
				continue;
			}

			//�ܺã�����ƥ��һ���ڲ��ķ�������
			if (CanChangeToZeroMem(listCode[szLast - 1].enSymbol) &&
				listCode[szLast - 1].u8CalcValue == 1)//���뱣֤u8CalcValue��1��������ܵ���[++]֮��Ĳ���������Ԥ��
			{
				//�ǳ������������Ҫ�Ż��Ĳ���
				bIsOptimization = true;//���һ�³ɹ�����������һ���Ż�

				//�ڴ沼�־������£�
				//>  [  +  ]  x  x  <
				//3  4  5  6  7  8  9
				//   ^     ^     ^
				//  top   lst   cur 

				//����szNewLast����֮ǰ��top
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
		//����Ҫ�պ�������
		MyAssert(codeBlockStack.empty(), "�Ż�ʧ�ܣ����Ų�ƥ�䣡");

		//��ɣ��ü����뵽szLast��λ�ã�������szLast�±��Ԫ�أ���Ϊfor���ᳬǰ����һ�Σ������ܰ�������֮ǰ��szLast��
		//��ע�⣬����������ڱ�Ҳ�����ڲ�����Ȼ���ƶ��������ٴδ���
		listCode.resize(szLast);

		return bIsOptimization;
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
	static bool OperatorMergeOptimization(CodeList &listCode)//�����Ƿ����������1���Ż�
	{
		if (!ContainsEnoughCode(listCode))//Ԫ�ز��㣬�����Ż���ֱ���˳�
		{
			return false;
		}

		bool bIsOptimization = false;

		//���˱�Ȼ������2Ԫ��
		size_t szLast = 0;
		for (size_t szCurrent = 1, szCodeSize = listCode.size(); szCurrent < szCodeSize; ++szCurrent)
		{
			//���Ժϲ�
			auto &cuLast = listCode[szLast];
			auto &cuCurrent = listCode[szCurrent];
			
			if (CanMergeOperator(cuLast.enSymbol, cuCurrent.enSymbol))//�ϲ������
			{
				bIsOptimization = true;//���һ��
				if (MergeOperator(cuLast.enSymbol, cuLast.u8CalcValue, cuCurrent.enSymbol, cuCurrent.u8CalcValue))
				{
					continue;//�����ϲ�
				}
				//�ϲ����������ⲿ����
			}
			else if (CanMergePointerMove(cuLast.enSymbol, cuCurrent.enSymbol))//�ϲ�ָ���ƶ�
			{
				bIsOptimization = true;//���һ��
				if (MergeOperator(cuLast.enSymbol, cuLast.szMovOffset, cuCurrent.enSymbol, cuCurrent.szMovOffset))
				{
					continue;//�����ϲ�
				}
				//�ϲ����������ⲿ����
			}
			else if (HasDuplicates(cuLast.enSymbol, cuCurrent.enSymbol))//ȥ�ز���
			{
				bIsOptimization = true;//���һ��
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

		//��ɣ��ü����뵽szLast + 1��λ�ã�����szLast�±��Ԫ�أ����ﲻ����for�����ģ���Ҫ�ֶ�+1�Ա㱣��szLast��Ԫ�أ�
		//��ע�⣬����������ڱ�Ҳ�����ڲ�����Ȼ���ƶ��������ٴδ���
		listCode.resize(szLast + 1);

		return bIsOptimization;
	}



	/*
	��Чѭ���Ż����жϵ���0���ѭ����ɾ����������û�õ�
	�������Ƕ��ѭ��ɾ����[[[[[++>++<<--]]]]]��Ϊ[++>++<<--]
	ɾ��������жϣ�����ɾ������������ѭ��

	�Ż�˳��Ӧ�����Ż��������Ƕ�ף�Ȼ�����Ż�Z[xxxx]ΪZ
		��ע�ⲻ��ȥ��Z��������ڴ��ֵ�᲻����Ԥ�ڣ�

	�������ѶȽϴ���Ҫ����ģʽƥ��
	*/
	static bool InvalidLoopOptimization(CodeList &listCode)
	{
		if (!ContainsEnoughCode(listCode))//Ԫ�ز��㣬�����Ż���ֱ���˳�
		{
			return false;
		}

		//�����ջ���洢������
		std::vector<size_t> codeBlockStack;

		 bool bIsOptimization = false;

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

			MyAssert(!codeBlockStack.empty(), "�Ż�ʧ�ܣ������ջ����Ϊ�գ�");//��ô�������أ�����ǰ���Ѿ�ƥ��������أ�

			/*
			����ÿһ��LoopEnd�����codeBlockStack��֮��Ӧ��LoopBegǰһ��LoopBeg��������Ϊ1�����������LoopBeg��
			������ǰ��ȡ1��CodeUnit���鿴�Ƿ�ΪLoopEnd�����ǣ�������codeBlockStack����ǰ�鿴��һ��LoopBeg�Ƿ������ڵ�LoopBeg���Ϊ1��
			�Һ�һ��CodeUnit��LoopEnd���������ѭ����ֱ��ĳһ���������㣬�ƶ�����ƥ�䵽�����ڲ�ѭ����codeBlockStack���������Ҫɾ�������
			����szLast�ƶ������ڲ��LoopEnd���ƶ�������λ�ã�ע�ⲻ�����λ�õ�֮��һ������Ϊfor�����������æ�ƶ�����
			�������Ƕ������ѭ���Ż�
			
			ͬʱ�������ǰLoopEnd��Ӧ��LoopBegǰһ��CodeUnitΪZeroMem����ɾ������ѭ����ע����Ż���ǰ���Ż������
			
			���������
			�ж������Ѿ���Ե�ѭ��ͷǰ��β���Ƿ������ڵ�ѭ�����ǣ������ж�ֱ��ȫ���Ż�
			Ȼ�����ƶ�����ǰ�����һ���ͷ���ǲ���ZeroMem������ǣ�ȫ��������
			szLast�ƶ���ZeroMemǰһ����Ԫ������޷��ƶ����Ѿ���0��������
			������ǰszCurrent+1λ�õĴ��룬���szCurrent+1 >= szCodeSize��
			ֱ�Ӵ���MyAssert����Ϊ����Ҫ��һ��ĩβ�����ַ�ProgEnd���ڣ��˴���Ԥ�ڣ��������
			*/

			//��ȡջ����������ʼ�������
			//�˴����ٱ�֤ջ����1��Ԫ��
			size_t szStackTop = codeBlockStack.size() - 1;
			
			//��һ��ѭ��ͷ��ѭ���ڱ�Ȼ��ʼ�������︳ֵһ����ֵ���ڱ����������ֱ�ӱ�ը~����
			size_t szLastLoopBeg = (size_t)-1;//��Ϊ��Ҫ���ⲿ����ʵ���жϵ���Ҫ�Ż���λ�ã���������븲����Χ���������ڲ�����
			//������һ��Ԥ�ڵ�ѭ��βΪ��ǰ+1����Ϊ�ʼ�Ѿ��ƶ���szCurrent�ˣ�����ȷ����ѭ��β���Ż�ִ�е��ˣ��ж���һ��
			size_t szNextLoopEnd = szCurrent + 1;//��Ҫ������Χ��������Ϊ�Ż���Ҫ֪��������Χ
			while (true)
			{
				//һ��ʼ���е��˵�ʱ���Ѿ���ȷ����listCode[szCurrent]�ƶ�����listCode[szLast]��ΪCodeUnit::Symbol::LoopEnd��
				//��ô����ҲҪ��szNextLoopEnd = szCurrent + 1ΪProgEnd������szNextLoopEnd >= szCodeSize�����
				//�����κ�����£������listCode[szNextLoopEnd].enSymbol != CodeUnit::Symbol::LoopEnd������LoopEnd�����˳�
				//���Դ�Assert����Ҫ�ɹ�����������˳�
				MyAssert(szNextLoopEnd < szCodeSize, "�Ż�ʧ�ܣ���ʧ������ǣ�");//���ǰɸ��ǣ��ҵ�ProgEndȥ�����ˣ�
				
				//����Ҫ���ж�֮ǰ��ֵ��ѭ����ʼ�ռ���szLastLoopBeg��codeBlockStack[szStackTop]
				szLastLoopBeg = codeBlockStack[szStackTop];

				//�����ʱ��ջ�������Ѿ���0������ǰ��û���κο������ĵ������ˣ���ô������������
				if (szStackTop == 0)
				{
					break;
				}
				
				size_t szPrevLoopBeg = codeBlockStack[szStackTop - 1];

				//���ǰѭ�����Ƿ����ڣ���һ��CodeUnit�Ƿ�ΪLoopEnd
				//����Ҫ����ѭ�����Ƿ����ڣ���Ϊ��szNextLoopEnd = szLastLoopEnd + 1��Ȼ����
				//����Ҫ���ǰһ��CodeUnit�Ƿ�ΪLoopBeg��ΪcodeBlockStack�ڱ�ΪLoopBeg
				//����ж��൱���ڲ鿴�Ƿ�Ϊ��Ե�����ѭ��
				if (szLastLoopBeg - szPrevLoopBeg != 1 ||
					listCode[szNextLoopEnd].enSymbol != CodeUnit::Symbol::LoopEnd)//���߼�������Ƕ�ײ�
				{
					break;//������
				}

				//���У���ô������Լ�����������
				//������һ��
				--szStackTop;
				++szNextLoopEnd;
			}

			//�ǵ��Ƴ�codeBlockStack�б��Ż���loopbeg!!!
			//�����Ƿ�����Ż���������ڿ�ʼ�жϸ�ǰ����û��ZEROMEMORY��־��
			//�еĻ�����������򵥸�ѭ���Ϳ���ȫɾ�ˣ���ȫ����ִ�еģ�����

			//����if��ȫɾ�ˣ����ǲ������ɾ�����ƶ�һ��szLast��ZeroMem��λ�ã�����ͬʱ�ƶ�szCurrent��������ɾ��
			if ((szStackTop == 0 && szLastLoopBeg == 0)||//ѭ����ͷ���ʼ��Ĭ���ڴ浥Ԫ��ʼֵΪ0��ѭ��Ҳ�൱�ڴ�ZeroMem��ʼ����ȫ������
				szLastLoopBeg != 0 && listCode[szLastLoopBeg - 1].enSymbol == CodeUnit::Symbol::ZeroMem)//�ж�szLastLoopBeg - 1Ҳ���ǵ�һ��szLastLoopBeg - szPrevLoopBeg != 1����µ�szLastLoopBegǰ���Ƿ��ж���
			{
				//�ܺã������Ż�
				bIsOptimization = true;//���һ��

				//����listCode����������ʾ��*��ʾ��������ʲô��x��ʾ��Ч����
				//szCurrentָ�����ʵҲ�Ǳ�move����Ч����ԭ�ȵ�ֵΪszLastָ���λ�ã�
			//    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  G  H  ...
			//    [  [  [  >  +  <  -  ]  x  x  x  x  ]  ]  *  *  *  *  ...
			//    ^                    ^           ^        ^   
			//    |                    |           |        |
			//szLastLoopBeg          szLast    szCurrent    |
			//codeBlockStack.back()                   szNextLoopEnd
			//                                        

			//�ƶ����Ч��Ӧ��������ʾ��ע������@��ʾ����������ʣ�����λ��
			//��ȻҲ�п�������һ�����������szLastLoopBegǰ�治��-1��ΪZeroMem
			//    -1  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  G  H  ...
			//     @  x  x  x  x  x  x  x  x  x  x  x  x  x  *  *  *  ...
			//     ^  ^                                   ^  ^
			//     |  |                                   |  |
			//  szLastLoopBeg                         szCurrent
			//     |                                         |
			//   szLast                                szNextLoopEnd
			//
			//codeBlockStack.empty() == true �� codeBlockStack.back() �� szLastLoopBeg ǰ��һ��[��λ��

				//szLastLoopBeg - 1��λ�øպ���ZeroMem��λ�ã���֤continue֮���ƶ�����һ����������
				//���szStackTop����ʵ����szLast = -1��һ��ѭ����ʱ��++��������Ϊ0��Ҳ������
				szLast = szLastLoopBeg - 1;
				
				//�ƶ�szCurrent��szNextLoopEnd - 1��Ҳ���ǵ�һ��ƥ��ѭ��βʧ�ܵ�λ��ǰ�棬
				//ע��������if��szStackTop == 0�˳������ģ���szNextLoopEnd��++֮ǰȷʵ��LoopEnd��
				//����szNextLoopEnd - 1���ǵ�һ��ƥ��ѭ��βʧ�ܵ�λ��ǰ��
				//��֤continue֮���ƶ�����һ����������
				szCurrent = szNextLoopEnd - 1;

				//��������֮����һ��ѭ����Ȼ��++szCurrent == szNextLoopEnd�ϵ�Ԫ�أ�
				//������listCode[++szLast]��

				//ƽ��ջֱ��szStackTop//Ϊʲô����Ҫ-1����Ϊֻ��Ҫɾ����szStackTopΪֹ��size�Ǳ�������1�ģ�
				//ɾ����������Ҳ���ǵ�ǰ��������size��������£���ǰ�����ϵ�Ԫ��Ҳ��ɾ�������-1���ɾ��1��
				//�������szStackTop == 0���պ��൱��ȫɾ�ˣ�ֱ��ѭ����ͷ
				codeBlockStack.resize(szStackTop);

				continue;//����forѭ��
			}

			//������˵�����ٴ�����һ������ѭ��ͷ����һ������ѭ��β��Ҳ���Ƿ�����ѭ��
			//����Ҳ��������Чѭ���Ż�����
			//���һ���Ƿ������ͷβ�ƶ����������ȷʵû���ƶ���˵��û��ƥ��ɹ��������Ż�
			if (szStackTop == codeBlockStack.size() - 1)//��ȫû���ƶ�
			{
				//����жϱ�����ǰ����Ч�Ż�֮������ÿ��ѭ�����ܳԵ���Ч�Ż����ԣ������Ǳ�����
				codeBlockStack.pop_back();//�ǵõ�����ǰƥ�䵽��loopbeg��ƽջ
				continue;//����forѭ��
			}

			//�����Ȼ���Ż�����
			bIsOptimization = true;//���һ��
			
			//�ܺã������ƶ���
			//����listCode����������ʾ��*��ʾ��������ʲô��x��ʾ��Ч����
			//szCurrentָ�����ʵҲ�Ǳ�move����Ч����ԭ�ȵ�ֵΪszLastָ���λ�ã�
		//    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  G  H  ...
		//    *  [  [  [  [  >  +  <  -  ]  x  x  x  ]  ]  *  *  ]  ...
		//          ^     ^              ^        ^        ^   
		//          |     |              |        |        |
		//    szLastLoopBeg            szLast     |        |
		//                |                   szCurrent    |
		//      codeBlockStack.back()                szNextLoopEnd

			//�ƶ����Ч��Ӧ��������ʾ
		//    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  G  H  ...
		//    *  [  [  >  +  <  -  ]  x  x  x  x  x  x  x  *  *  ]  ...
		//    	 ^  ^              ^                    ^  ^
		//       |  |              |                    |  |
		//    szLastLoopBeg      szLast             szCurrent
		//       |                                         |
		//codeBlockStack.back()                      szNextLoopEnd

			size_t szNearLoopBeg = codeBlockStack.back();
			for (size_t szSource = szNearLoopBeg,
						szSourceEnd = szLast + 1,
						szTarget = szLastLoopBeg;
				szSource < szSourceEnd; ++szSource, ++szTarget)
			{
				listCode[szTarget] = std::move(listCode[szSource]);//�ƶ������ڵĴ���
			}

			//���õ�ǰλ������һλ��
			szCurrent = szNextLoopEnd - 1;
			szLast -= szNearLoopBeg;

			//����ջ����СΪszStackTop�൱����codeBlockStack.back()��szLastLoopBeg��ǰ��һ��[λ��
			codeBlockStack.resize(szStackTop);

			continue;//����ƶ�������forѭ��
		}
		//����Ҫ�պ�������
		MyAssert(codeBlockStack.empty(), "�Ż�ʧ�ܣ����Ų�ƥ�䣡");

		//��ɣ��ü����뵽szLast��λ�ã�������szLast�±��Ԫ�أ���Ϊfor���ᳬǰ����һ�Σ������ܰ�������֮ǰ��szLast��
		//��ע�⣬����������ڱ�Ҳ�����ڲ�����Ȼ���ƶ��������ٴδ���
		listCode.resize(szLast);

		return bIsOptimization;
	}


	/*
	ѭ����ת�������Ż���
	ǰ���Ż���ɺ���������ѭ������ת�㣬ͬʱ�Ż�ͬβѭ����ͷ��ת��Ϊ���һ����ѭ����β��
	ע��˺������践���Ƿ�������Ż���ֱ�ӷ���
	*/

	static void LoopSettingAndOptimization(CodeList &listCode)
	{
		if (!ContainsEnoughCode(listCode))//Ԫ�ز��㣬�����Ż���ֱ���˳�
		{
			return;
		}

		//�����ջ���洢������
		std::vector<size_t> codeBlockStack;

		//��������listCode�����ı�ѭ������
		for (size_t szCurrent = 0, szCodeSize = listCode.size(); szCurrent < szCodeSize; ++szCurrent)
		{
			if (listCode[szCurrent].enSymbol == CodeUnit::Symbol::LoopBeg)
			{
				codeBlockStack.push_back(szCurrent);
				continue;
			}

			if (listCode[szCurrent].enSymbol != CodeUnit::Symbol::LoopEnd)
			{
				continue;
			}

			MyAssert(!codeBlockStack.empty(), "�Ż�ʧ�ܣ������ջ����Ϊ�գ�");

			//��������һ����֮ƥ���LoopEnd�������������������LoopEndֱ������һ����LoopEnd��
			//��������ǰ����Ե�LoopBeg���Ǹ���LoopEnd��λ�ã�����Ż�
			size_t szStackTop = codeBlockStack.size() - 1;//��ȡջ����ջ���뵱ǰƥ��
			//�����õ�ǰָ���loopend��ջ��loopbeg����ת��ϵ
			listCode[szCurrent].szJmpIndex = codeBlockStack[szStackTop];

			//szNewCurrent����������ѭ����β��λ��
			size_t szNewCurrent = szCurrent;
			while (true)
			{
				//ʵ���ϣ�������szNewCurrent + 1 >= szCodeSize��
				//��Ϊ������һ����LoopEnd����ô��Ȼ���������ַ���
				//��������֮ǰ������β����ҪAssert��֤
				MyAssert(szNewCurrent + 1 < szCodeSize, "�Ż�ʧ�ܣ���ʧ������ǣ�");

				//�����ǰ���ظ����˳�
				if(listCode[++szNewCurrent].enSymbol != CodeUnit::Symbol::LoopEnd)
				{
					break;
				}
			}

			//������ظ��ĸ�������������ǰ����-1��
			size_t szRepetitions = szNewCurrent - szCurrent - 1;
			size_t szStackSize = codeBlockStack.size();
			//size - 1��ȡtop����Ȼ��-szRepetitions��ȡ�ظ���ʼʱ����Ե�loopbeg����
			size_t szStackOptimizationBeg = szStackSize - 1 - szRepetitions;

			//size��Ҫ-1��ԭ����size��������˵�ǰƥ���loopend�����ظ���loopend��СӦ���ų���ǰloopend
			//���szRepetitions == codeBlockStack.size() - 1�����պ�ƥ��
			MyAssert(szRepetitions <= codeBlockStack.size() - 1, "�Ż�ʧ�ܣ����Ų�ƥ�䣡");

			//����szNewCurrent��λ�þ�������ѭ����Ҫ��β��λ��
			for (size_t szIndex = szStackOptimizationBeg; szIndex < szStackSize; ++szIndex)
			{
				size_t szCodeListIndex = codeBlockStack[szIndex];
				listCode[szCodeListIndex].szJmpIndex = szNewCurrent;
			}

			//����ֱ��szStackOptimizationBeg��λ�ã�ע�������᲻������ǰszStackOptimizationBegָ���ֵ
			codeBlockStack.resize(szStackOptimizationBeg);

			continue;//�����ɣ�����ѭ��
		}
		//����Ҫ�պ�������
		MyAssert(codeBlockStack.empty(), "�Ż�ʧ�ܣ����Ų�ƥ�䣡");

		return;
	}



	static void Optimization(CodeList &listCode)
	{
		//��Ҫ�޸�Ϊ�����Ƿ����������һ���Ż�
		//Ȼ��˳�����У�ֱ��ĳһ�����еĺ���
		//������false������û�и�������Ż�����

		//��Ϊ����Ҫ����һ���Ż��ұ���Ҫ��֤�Ż���˳�����ֱ��ȫ��false
		//���Ա����·��ֵ���⣬ʹ��boolֵ����
		PrintCodeList(listCode, "PreOptimization:", "\n");

		printf("while\n");
		while (true)
		{
			bool b0 = CountdownZeroOptimization(listCode);//���Ż�������ƥ��Ĺ̶�ģʽ
			PrintCodeList(listCode, "CountdownZeroOptimization:", "\n");
			bool b1 = OperatorMergeOptimization(listCode);//���Ž��в���ȥ���Ż�
			PrintCodeList(listCode, "OperatorMergeOptimization:", "\n");
			bool b2 = InvalidLoopOptimization(listCode);//Ȼ���Ż�����Чѭ��
			PrintCodeList(listCode, "InvalidLoopOptimization:", "\n");

			if (b0 || b1 || b2)//ֻҪ������һ���ɹ�����ô����
			{
				printf("continue\n");
				continue;
			}

			//����ֱ������
			break;
		}
		printf("break\n");

		LoopSettingAndOptimization(listCode);//������ѭ����תλ��ƥ��������β����ת�Ż�
		PrintCodeList(listCode, "LoopSettingAndOptimization:", "\n");
		return;
	}


public:

	//����ú�������false����ô�κ�ʹ��listCode����ִ�еĲ�������δ������Ϊ����Ȼ����ȡʧ���ֳ���û�����
	static bool PreprocessInStream(StreamType &sStream, CodeList &listCode, bool bIgnoreUnknownChar, bool bOptimization)
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

		//�������һ��Ҫ�ٴμ��ĩβ�Ƿ���ProgEnd�����û�У�������Ҫǿ���޸���˵���ڲ���������
		//����ʹ��for��·��ֵ������Ѿ�Ϊ���򲻻����back����out of rangeը��
		if (listCode.empty() || listCode.back().enSymbol != CodeUnit::Symbol::ProgEnd)
		{
			printf("Ԥ������󣺴�������Ϊ�ջ�ʧ������ǣ�\n");
			return false;
		}

		return true;
	}
	//Ԥ�����������д�����

	//Ԥ�������Ż��д����ƣ�����ʶ�𾭵��0�������ƶ��������ݼ�����������Ƕ�׵����ȣ�


	/*
	TODO:Ԥ������CodeUnit���浽�ļ�����׺.bfvc��brainfuck virtual code
	ִ������ʱ����ʶ���ļ����ͣ�bfvc�����ļ�ͷ��BFVC��ͷ������ļ���Ϣ��
		����.bfԴ���룬��������ת�������ݾ�����Ҫ�����Ż�ִ��
		.bfvc�����У�Ҳ���ܽ����κ��Ż�������Ҫ��������ļ��Ϸ��ԣ�������Ե�
	֧��һ���̶ȵ�bfvc�����뵽bf�����㶮�ģ��ظ�һ��code unit��ZRM�滻Ϊ[-]֮��ģ�
	*/

};