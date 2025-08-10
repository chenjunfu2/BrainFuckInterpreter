#pragma once

#include "CodeUnit.hpp"
#include "FileStream.hpp"
#include "StrStream.hpp"

#include <cstdio>
#include <vector>
#include <cctype>

template<typename StreamType>
class Preprocessor//预处理器
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
	//			break;//文件结束，离开
	//		}
	//
	//		//判断是否相同（处理重复字符）
	//		if (cRead != cFind)
	//		{
	//			sStream.UnGet();//回退1读取
	//			break;//遇到不同，直接离开
	//		}
	//		
	//		++szDupCount;//相同继续合并
	//	}
	//
	//	return szDupCount;
	//}

	static bool ZeroPreprocess(StreamType &sStream, CodeList &listCode, bool bIgnoreUnknownChar = false)
	{
		if (!sStream)//文件是NULL，返回
		{
			return false;
		}

		//块语句栈（存储索引）
		std::vector<size_t> codeBlockStack;

		listCode.clear();//清理以便读取

		//报错信息
		size_t szLine = 1;
		size_t szColumn = 1;

		//读取并转换到代码列表
		while (true)
		{
			char cRead = sStream.GetChar();
			if (sStream.Eof())
			{
				break;//读完离开
			}

			//每读一个字符增加一列
			++szColumn;//处理列号

			//第0次预处理不进行任何优化，用于debug与报错输出
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
					CurCode.enSymbol = CodeUnit::OptCur;//仅保存指令符号，附加信息无效
				}
				break;
			case ',':
				{
					CurCode.enSymbol = CodeUnit::IptCur;//仅保存指令符号，附加信息无效
				}
				break;
			case '['://先预留，并压入栈，等待]计算跳转偏移量
				{
					CurCode.enSymbol = CodeUnit::LoopBeg;
					listCode.push_back(CurCode);//提前压入
					codeBlockStack.push_back(listCode.size() - 1);//保存索引以便后续修改
				}
				continue;//注意此处为continue而非break，不走下面默认压入
			case ']':
				{
					if (codeBlockStack.empty())//什么，根本没有左括号，哪来的右括号，去你的
					{
						printf("解析失败[line:%zu,column:%zu]：括号未匹配\n", szLine, szColumn);
						return false;
					}

					//找到一个loopend，寻找最近的配对：
					//获取栈顶（也就是最近一个与当前匹配loopbeg）然后弹出
					size_t listCodeIndex = codeBlockStack.back();
					codeBlockStack.pop_back();

					CurCode.enSymbol = CodeUnit::LoopEnd;
					CurCode.szJmpIndex = listCodeIndex;//存入跳转位置，让当前能跳转到loopbeg
					listCode.push_back(CurCode);//提前插入，以便更新size

					//设置loopbeg跳转目标位置为当前之后，这样当条件不满足后跳过循环（因为当前已经插入，所以size刚好是尾后index）
					listCode[listCodeIndex].szJmpIndex = listCode.size();
				}
				continue;//注意此处为continue而非break，不走下面默认压入
			case '?':
				{
					CurCode.enSymbol = CodeUnit::DbgInfo;//仅保存指令符号，附加信息无效
				}
				break;
			case '#':
				{
					//处理注释
					//读取，直到换行符中的任意一个，回退并走default处理，以便统一行号列号
					while (true)
					{
						char cRead = sStream.GetChar();
						if (sStream.Eof())
						{
							break;//离开循环，回到外层判断，并退出
						}

						if (cRead == '\r' || cRead == '\n')
						{
							sStream.UnGet();//回退1字节，退出循环
							break;
						}
						//否则丢弃读取的字符（注释无需保留）
					}
				}
				continue;//注意此处为continue而非break，不走下面默认压入
			default:
				{
					//处理换行：只存在：\r或\n或\r\n的情况，无其他情况
					bool bNewLine = false;
					if (cRead == '\r')//判断下一个是不是\n
					{
						bNewLine = true;
						cRead = sStream.GetChar();
						if (!sStream.Eof() && cRead != '\n')
						{
							sStream.UnGet();//回退1字节
						}//此处哪怕eof也不用担心，更新后就会continue，然后退出循环
					}
					else if (cRead == '\n')
					{
						bNewLine = true;
					}

					if (bNewLine)
					{
						++szLine;//递增行号
						szColumn = 1;//重置列号
						continue;//直接继续
					}

					if (bIgnoreUnknownChar)
					{
						continue;//直接全部跳过，不判断空白还是未知字符了
					}

					//不是换行，处理其余空白
					if (isspace(cRead))
					{
						continue;//跳过空白，直接继续
					}

					//都不是，那么未知字符且不在注释内，报错
					printf("解析失败[line:%zu,column:%zu]：遇到未知字符\'%c\'\n", szLine, szColumn, cRead);
				}
				return false;//注意报错直接返回
			}

			//压入列表
			listCode.push_back(CurCode);
		}

		//判断栈内是否还有残留的括号，是，则代表括号未配对，报错
		if (!codeBlockStack.empty())
		{
			printf("解析失败[line:%zu,column:%zu]：括号未匹配\n", szLine, szColumn);
			return false;
		}

		//完成，尾部插入一个卫兵字符，也就是ProgEnd，执行到代表程序结束
		//同时能确保循环跳转到末尾能正确处理而不是index out of range
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
		return	IsOperator(enTargetiSym) && IsOperator(enSourceSym);//都是运算符
	}
	static bool CanMergePointerMove(CodeUnit::Symbol enTargetiSym, CodeUnit::Symbol enSourceSym)
	{
		return IsPointerMove(enTargetiSym) && IsPointerMove(enSourceSym);//都是指针操作
	}


	static bool FirstOptimization(CodeList &listCode)//返回是否进行了至少1次优化
	{
		bool bIsOpti = false;

		CodeList listOptiCode{};
		bool bPushNew = true;//第一下让它插入一个元素防止listOptiCode.back炸掉
		for (auto &it : listCode)//补药再说什么size循环调用有开销了，编译器能优化的啊哥哥
		{
			if (bPushNew)
			{
				listOptiCode.push_back(it);//插入一个新的并迭代到下一个
				bPushNew = false;
				continue;
			}

			//尝试合并
			auto &cuLast = listOptiCode.back();
			
			if (CanMergeOperator(cuLast.enSymbol, it.enSymbol))//合并运算符
			{
				bIsOpti = true;
				if (cuLast.enSymbol != it.enSymbol)//符号不同，比大小
				{
					if (cuLast.u8CalcValue > it.u8CalcValue)//当前更大
					{
						cuLast.u8CalcValue -= it.u8CalcValue;//当前异号直接做差，不需要注意正负（数学可证）
					}
					else if(cuLast.u8CalcValue < it.u8CalcValue)//当前更小（需要反转）
					{
						cuLast.u8CalcValue = it.u8CalcValue - cuLast.u8CalcValue;
						cuLast.enSymbol = it.enSymbol;
					}
					else//相等且异号，直接消除
					{
						listOptiCode.pop_back();//删除末尾且不插入当前值
						bPushNew = true;
					}
				}
				else//符号相同，直接相加
				{
					cuLast.u8CalcValue += it.u8CalcValue;
				}
			}
			else if (CanMergePointerMove(cuLast.enSymbol, it.enSymbol))//合并指针移动
			{
				bIsOpti = true;
				if (cuLast.enSymbol != it.enSymbol)//符号不同，比大小
				{
					if (cuLast.szMovOffset > it.szMovOffset)//当前更大
					{
						cuLast.szMovOffset -= it.szMovOffset;//当前异号直接做差，不需要注意正负（数学可证）
					}
					else if (cuLast.szMovOffset < it.szMovOffset)//当前更小（需要反转）
					{
						cuLast.szMovOffset = it.szMovOffset - cuLast.szMovOffset;
						cuLast.enSymbol = it.enSymbol;
					}
					else//相等且异号，直接消除
					{
						listOptiCode.pop_back();//删除末尾且不插入当前值
						bPushNew = true;
					}
				}
				else//符号相同，直接相加
				{
					cuLast.szMovOffset += it.szMovOffset;
				}
			}
			else//无法合并
			{
				listOptiCode.push_back(it);//直接插入，注意不要设置标志位，以便下一个循环判断
			}
		}

		//完成
		if (bIsOpti)//至少进行一次优化，这覆盖原先的代码
		{
			listCode = std::move(listOptiCode);
		}//如果没有任何优化，直接自动析构

		return bIsOpti;
	}

	static bool SecondOptimization(CodeList &listCode)//返回是否进行了至少1次优化
	{





	}





	static bool FirstPreprocess(CodeList &listCode)//返回是否成功
	{

	}

	static bool SecondPreprocess(CodeList &listCode)//返回是否成功
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

	//如果该函数返回false，那么任何使用listCode进行执行的操作都是未定义行为，当然，读取失败现场是没问题的
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
	//预处理器报错有待完善

	//预处理器优化有待完善（比如识别经典归0，来回移动，递增递减抵消，无用嵌套抵消等）

};


/*
改为多次解析：
- 第零次预处理：无优化方便报错
- 第一次预处理：循环调用第一次优化
- 第二次预处理：调用第二次优化后循环调用1~2直到全部完成


- 第一次优化：合并连续的所有运算操作或所有指针操作，
	比如+++变为+3，同时+3 -1变成+2，>>>变成>3，同时>3 <3直接抵消，
	因为存在抵消导致合并其他项可以继续优化的情况，所以需要循环直到无法优化为止
- 第二次优化：处理连续循环结尾，去掉所有诸如[-]、[+]的归零操作，变为直接操作
	此操作后可能导致新的合并，需要重复第一次优化和本次优化

*/