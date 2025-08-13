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

	static bool NoOptimizationPreprocess(StreamType &sStream, CodeList &listCode, bool bIgnoreUnknownChar)
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


	//判断是否拥有足够多的打码来进行优化，至少2个。为什么要+1？因为有结尾哨兵标记ProgEnd占用1空间
	static bool ContainsEnoughCode(CodeList &listCode)
	{
		return listCode.size() < 2 + 1;
	}
	static bool IsOperator(CodeUnit::Symbol enSym)//判断是否是数值运算类型
	{
		return enSym == CodeUnit::Symbol::AddCur || enSym == CodeUnit::Symbol::SubCur;
	}
	static bool IsPointerMove(CodeUnit::Symbol enSym)//判断是否是指针运算类型
	{
		return enSym == CodeUnit::Symbol::NextMov || enSym == CodeUnit::Symbol::PrevMov;
	}
	static bool IsZeroMem(CodeUnit::Symbol enSym)
	{
		return enSym == CodeUnit::Symbol::ZeroMem;
	}
	static bool IsDuplicateRemoval(CodeUnit::Symbol enSym)//判断是否是需要去重的代码类型
	{
		return IsZeroMem(enSym);
	}

	static bool CanChangeToZeroMem(CodeUnit::Symbol enSym)//判断是否是需要优化为内存置0操作
	{
		return IsOperator(enSym) || IsZeroMem(enSym);
	}


	/*
	循环倒数置0优化：先匹配[-]或[+]形式，替换为Z，
	但是也要匹配[Z]，防止[[-]]之类的嵌套情况，要跑多次优化
	然后在此基础上调用OperatorMergeOptimization去掉重复的ZZZZ以及优化后可以合并的新的点
	*/
	static void CountdownZeroOptimization(CodeList &listCode)
	{
		if (!ContainsEnoughCode(listCode))//元素不足，无需优化，直接退出
		{
			return;
		}

		//块语句栈（存储索引）
		std::vector<size_t> codeBlockStack;

		size_t szLast = 0;//依然是快慢指针操作
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

			MyAssert(!codeBlockStack.empty(), "优化失败：块语句栈意外为空！");//怎么会这样呢？明明前面已经匹配完成了呢？

			//现在遇到了一个LoopEnd
			//查看codeBlockStack栈顶，检查与之匹配的LoopBeg索引与当前LoopEnd的差值
			//如果差值刚好为2则代表可能匹配到[x]序列，判断x的类型，然后决策
			if (szLast - codeBlockStack.back() != 2)
			{
				//并不是2，那么，弹出跳过
				codeBlockStack.pop_back();
				continue;
			}

			//很好，现在匹配一下内部的符号类型
			if (CanChangeToZeroMem(listCode[szLast - 1].enSymbol) &&
				listCode[szLast - 1].u8CalcValue == 1)//必须保证u8CalcValue是1，否则可能导致[++]之类的操作不符合预期
			{
				//非常棒，这就是需要优化的部分
				size_t szNewLast = codeBlockStack.back();
				codeBlockStack.pop_back();
				//内存布局举例如下：
				//>  [  +  ]  x  x  <
				//3  4  5  6  7  8  9
				//   ^     ^     ^
				//  top   lst   cur 

				//现在szNewLast就是top
				//将szNewLast的Symbol变为ZeroMem
				listCode[szNewLast].enSymbol = CodeUnit::Symbol::ZeroMem;
				listCode[szNewLast].szMovOffset = 0;//归零无用内存
				//把szLast变为NewLast：设置慢指针为优化位置
				szLast = szNewLast;

				//内存布局现在如下：
				//>  Z  x  x  x  x  <
				//3  4  5  6  7  8  9
				//   ^           ^
				//  lst         cur 
				//for在下一次递增的时候会执行操作移动到下一位并拷贝
				continue;
			}
		}
	}

	static bool CanMergeOperator(CodeUnit::Symbol enTargetiSym, CodeUnit::Symbol enSourceSym)
	{
		return	IsOperator(enTargetiSym) && IsOperator(enSourceSym);//都是运算符
	}
	static bool CanMergePointerMove(CodeUnit::Symbol enTargetiSym, CodeUnit::Symbol enSourceSym)
	{
		return IsPointerMove(enTargetiSym) && IsPointerMove(enSourceSym);//都是指针操作
	}
	static bool HasDuplicates(CodeUnit::Symbol enTargetiSym, CodeUnit::Symbol enSourceSym)
	{
		return IsDuplicateRemoval(enTargetiSym) && enTargetiSym == enSourceSym;//都是需要去重的类型并且相等
	}


	//返回值代表是否还需要保留两值（如果刚好相反合并后为0，则返回false表示可以丢弃合并）
	template<typename T>
	static bool MergeOperator(CodeUnit::Symbol &enSymLeft, T &tMergeLeft, CodeUnit::Symbol &enSymRight, T &tMergeRight)
	{
		if (enSymLeft == enSymRight)//符号相同，直接相加
		{
			tMergeLeft += tMergeRight;
			return true;
		}

		//符号不同，比大小
		if (tMergeLeft > tMergeRight)//当前更大
		{
			tMergeLeft -= tMergeRight;//当前异号直接做差，不需要注意正负（数学可证）
		}
		else if (tMergeLeft < tMergeRight)//当前更小（需要反转）
		{
			tMergeLeft = tMergeRight - tMergeLeft;
			enSymLeft = enSymRight;//改变符号
		}
		else//相等且异号，直接消除
		{
			return false;
		}

		return true;
	}

	/*
	合并优化原理：合并多个同类运算符的多次相同操作
	在源对象上操作，利用快慢索引，快索引遍历代码单元
	慢索引每次尝试合并快索引上的同类单元，成功慢索引不变
	否则慢索引递增并拷贝快索引上的单元到慢索引位置，
	快慢索引之间的区域就是被优化掉的区域，慢索引会慢慢往后覆盖，
	直到快索引遇到代码单元列表的结尾，裁切直到慢索引的位置，优化结束
	*/
	static void OperatorMergeOptimization(CodeList &listCode)//返回是否进行了至少1次优化
	{
		if (!ContainsEnoughCode(listCode))//元素不足，无需优化，直接退出
		{
			return;
		}

		//到此必然有至少2元素
		size_t szLast = 0;
		for (size_t szCurrent = 1, szCodeSize = listCode.size(); szCurrent < szCodeSize; ++szCurrent)
		{
			//尝试合并
			auto &cuLast = listCode[szLast];
			auto &cuCurrent = listCode[szCurrent];
			
			if (CanMergeOperator(cuLast.enSymbol, cuCurrent.enSymbol))//合并运算符
			{
				if (MergeOperator(cuLast.enSymbol, cuLast.u8CalcValue, cuCurrent.enSymbol, cuCurrent.u8CalcValue))
				{
					continue;//正常合并
				}
				//合并抵消，走外部处理
			}
			else if (CanMergePointerMove(cuLast.enSymbol, cuCurrent.enSymbol))//合并指针移动
			{
				if (MergeOperator(cuLast.enSymbol, cuLast.szMovOffset, cuCurrent.enSymbol, cuCurrent.szMovOffset))
				{
					continue;//正常合并
				}
				//合并抵消，走外部处理
			}
			else if (HasDuplicates(cuLast.enSymbol, cuCurrent.enSymbol))//去重操作
			{
				continue;//直接继续直到任意一个不匹配
			}
			else//无法合并
			{
				//移动并处理下一个
				listCode[++szLast] = std::move(listCode[szCurrent]);
				continue;//注意这里必须提前continue，不需要走下面的合并抵消处理
			}

			//合并后抵消，需要丢弃值
			if (szLast > 0)
			{
				--szLast;//让慢索引移动到上一个单元（如果有），这样下一个快索引单元就能尝试与上一个合并
			}
			else//到头了，寄，只能尝试递增快索引并移动到当前（0）后继续
			{
				//确定一下是否还有东西可以移动
				//理论上因为末尾有哨兵，不太可能会有这种情况，但是以防万一，对吧
				MyAssert(szCurrent + 1 < szCodeSize, "优化错误：在结束标记前遇到末尾！");

				//移动当前的到头部，继续执行优化
				listCode[szLast] = std::move(listCode[++szCurrent]);
			}
		}

		//完成，裁剪代码到szLast + 1的位置（注意，正常情况下哨兵也会走内部处理然后移动，无需再次处理）
		listCode.resize(szLast + 1);
	}



	/*
	无效循环优化：判断到置0后的循环，删除它，这是没用的
	无意义的嵌套循环删除：[[[[[++>++<<--]]]]]变为[++>++<<--]
	删除后继续判断，可以删除连续的无用循环

	优化顺序：应当先优化无意义的嵌套，然后再优化Z[xxxx]为Z
		（注意不能去掉Z否则可能内存的值会不符合预期）

	本函数难度较大，需要多种模式匹配
	*/
	static void InvalidLoopOptimization(CodeList &listCode)
	{
		if (!ContainsEnoughCode(listCode))//元素不足，无需优化，直接退出
		{
			return;
		}

		//块语句栈（存储索引）
		std::vector<size_t> codeBlockStack;

		size_t szLast = 0;//依然是快慢指针操作
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

			MyAssert(!codeBlockStack.empty(), "优化失败：块语句栈意外为空！");//怎么会这样呢？明明前面已经匹配完成了呢？

			/*
			对于每一个LoopEnd，检测codeBlockStack与之对应的LoopBeg前一个LoopBeg，如果差距为1，则代表相邻LoopBeg，
			往后提前读取1个CodeUnit并查看是否为LoopEnd，若是，继续在codeBlockStack中往前查看下一个LoopBeg是否与现在的LoopBeg差距为1，
			且后一个CodeUnit是LoopEnd，是则继续循环，直到某一条件不满足，移动所有匹配到的最内层循环到codeBlockStack中最左侧需要删除的外层
			并将szLast移动到最内层的LoopEnd在移动后的最后位置（注意不是最后位置的之后一个，因为for会往后递增帮忙移动），
			完成连续嵌套无用循环优化
			
			同时，如果当前LoopEnd对应的LoopBeg前一个CodeUnit为ZeroMem，则删除整段循环，注意此优化在前面优化完后检测
			
			具体操作：
			判断现在已经配对的循环头前与尾后是否有相邻的循环，是，继续判断直到全部优化
			然后在移动代码前，检测一下最开头的是不是ZeroMem，如果是，全部丢弃，
			szLast移动到ZeroMem前一个单元，如果无法移动（已经是0索引），
			拷贝当前szCurrent+1位置的代码，如果szCurrent+1 >= szCodeSize，
			直接触发MyAssert，因为至少要有一个末尾卫兵字符ProgEnd存在，此处非预期，程序结束
			*/

			//获取栈顶索引，开始连续检测
			//此处至少保证栈顶有1个元素
			size_t szStackTop = codeBlockStack.size() - 1;
			
			//下一个循环头（循环内必然初始化，这里赋值一个极值用于避免误操作（直接爆炸~））
			size_t szLastLoopBeg = (size_t)-1;//因为需要在外部访问实际判断到需要优化的位置，作用域必须覆盖外围，不能在内侧声明
			//设置下一个预期的循环尾为当前+1，因为最开始已经移动过szCurrent了，并且确认是循环尾，才会执行到此，判断下一个
			size_t szNextLoopEnd = szCurrent + 1;//需要覆盖外围作用域，因为优化需要知道拷贝范围
			while (true)
			{
				//一开始运行到此的时候，已经能确认由listCode[szCurrent]移动到的listCode[szLast]必为CodeUnit::Symbol::LoopEnd，
				//那么至少也要是szNextLoopEnd = szCurrent + 1为ProgEnd而不是szNextLoopEnd >= szCodeSize溢出，
				//并且任何情况下，下面的listCode[szNextLoopEnd].enSymbol != CodeUnit::Symbol::LoopEnd遇到非LoopEnd都会退出
				//所以此Assert必须要成功，否则程序退出
				MyAssert(szNextLoopEnd < szCodeSize, "优化失败：丢失结束标记！");//不是吧哥们，我的ProgEnd去哪里了？
				

				//如果这时候栈顶已经是0，没有任何可以消耗的内容了，那么，可以跳过了
				//或者，如果szNewCurrent >= szCodeSize的话，后面也没有可以消耗的内容了，可以跳过了
				//这也解释了为什么前面szStackTop不是codeBlockStack.size() - 2而是- 1，
				//因为更前面的MyAssert(!codeBlockStack.empty(), ...);
				//只能保证有1单元，通过这里的检测后，才能往前继续访问1单元
				if (szStackTop == 0)
				{
					break;
				}

				szLastLoopBeg = codeBlockStack[szStackTop];
				size_t szPrevLoopBeg = codeBlockStack[szStackTop - 1];

				//检测前循环块是否相邻，后一个CodeUnit是否为LoopEnd
				//不需要检测后循环块是否相邻，因为是szNextLoopEnd = szLastLoopEnd + 1必然相邻
				//不需要检测前一个CodeUnit是否为LoopBeg因为codeBlockStack内必为LoopBeg
				//这个判断相当于在查看是否为配对的相邻循环
				if (szLastLoopBeg - szPrevLoopBeg != 1 ||
					listCode[szNextLoopEnd].enSymbol != CodeUnit::Symbol::LoopEnd)//反逻辑，减少嵌套层
				{
					break;//结束力
				}

				//命中，那么代表可以继续连续查找
				//访问下一个
				--szStackTop;
				++szNextLoopEnd;
			}

			//记得移出codeBlockStack中被优化的loopbeg!!!
			//不论是否存在优化情况，现在开始判断更前面有没有ZEROMEMORY标志，有的话，这个循环就可以全删了，完全不会执行的，哥们

			if (szStackTop == 0 && codeBlockStack[szStackTop] == 0)
			{//循环开头在最开始，默认内存单元初始值为0，循环也相当于从ZeroMem开始，完全被跳过

				szLast = -1;//下一次循环的时候++会让它变为0
				
				//移动szCurrent到szNextLoopEnd - 1，也就是第一个匹配循环尾失败的位置前面，
				//注意这个if是szStackTop == 0退出后进入的，而szNextLoopEnd在++之前确实是LoopEnd，
				//所以-1就是第一个匹配循环尾失败的位置前面
				szCurrent = szNextLoopEnd - 1;

				//这样操作之后，下一次循环必然有++szCurrent == szNextLoopEnd上的元素，
				//拷贝到listCode[++szLast]（szLast == 0）上

				//平栈
				codeBlockStack.clear();//全删了，直到循环开头

				continue;//继续循环
			}
			//判断szLastLoopBeg - 1也就是第一个szLastLoopBeg - szPrevLoopBeg != 1情况下的szLastLoopBeg前面是否有东西
			if (szLastLoopBeg != 0 &&
				listCode[szLastLoopBeg - 1].enSymbol == CodeUnit::Symbol::ZeroMem)
			{
				//下面不用移动优化了，全删了，但是不是真的删除，移动一下szLast到ZeroMem的位置，并且同时移动szCurrent即可懒惰删除

				//szLastLoopBeg - 1的位置刚好是ZeroMem的位置，保证continue之后移动到下一个正常处理
				szLast = szLastLoopBeg - 1;
				//szNextLoopEnd - 1刚好是上一个被使用的下标
				//保证continue之后移动到下一个正常处理
				szCurrent = szNextLoopEnd - 1;

				//平衡栈直到szStackTop//为什么不需要-1？因为只需要删除到szStackTop为止，size是比索引大1的，
				//删除到索引（也就是当前索引当成size）的情况下，当前索引上的元素也被删除，如果-1会多删除1个
				codeBlockStack.resize(szStackTop);

				continue;//继续for循环
			}
			



			//执行到此：前面有个p的东西，或者根本不是ZeroMem，尝试优化重复无效循环

			
			//先移动szCurrent到前面匹配到的szNextLoopEnd位置 - 1
			//后移动szLast到前面匹配到的szStackTop位置-1（如果有），
			//如果没有（szStackTop为0），那么尝试操作新的szCurrent + 1拷贝到当前
			//如果也没有（szCurrent + 1 >= szCodeSize），直接退出循环

			//到这里说明至少存在上一个不是循环头或下一个不是循环尾，也就是非相邻循环
			//检查一下是否进行了头尾移动操作，如果确实没有移动，说明没有匹配成功，无需优化
			if (szStackTop == )//完全没有移动
			{
				continue;//继续for循环
			}

			





		}





	}


	/*
	循环跳转设置与优化：
	前置优化完成后，设置所有循环的跳转点，同时优化同尾循环开头跳转点为最后一个外循环的尾部
	*/

	static void LoopSettingAndOptimization(CodeList &listCode)
	{
		if (!ContainsEnoughCode(listCode))//元素不足，无需优化，直接退出
		{
			return;
		}


	}



	static void Optimization(CodeList &listCode)
	{
		//需要修改为返回是否进行了至少一次优化
		//然后按顺序运行，直到某一遍所有的函数
		//都返回false，代表没有更多可以优化的了

		CountdownZeroOptimization(listCode);//先优化掉可以匹配的固定模式
		
		OperatorMergeOptimization(listCode);//接着进行操作去重优化
		InvalidLoopOptimization(listCode);//然后优化掉无效循环

		LoopSettingAndOptimization(listCode);//最后进行循环优化，同时设置循环跳转关系
	}


public:

	//如果该函数返回false，那么任何使用listCode进行执行的操作都是未定义行为，当然，读取失败现场是没问题的
	static bool PreprocessInStream(StreamType &sStream, CodeList &listCode, bool bIgnoreUnknownChar, bool bOptimization = true)
	{
		//先进性无优化预处理（调试分析用）
		if (!NoOptimizationPreprocess(sStream, listCode, bIgnoreUnknownChar))
		{
			return false;
		}

		//如果设置了优化，则跑优化处理
		if (bOptimization)
		{
			//注意：任何形式的优化都会导致循环跳转索引失效，所以Optimization会在最后重设
			Optimization(listCode);
		}

		//最后的最后，一定要再次检测末尾是否有ProgEnd，如果没有，报错，不要强制修复，说明内部出现问题
		//这里使用for短路求值，如果已经为空则不会访问back导致out of range炸掉
		if (listCode.empty() || listCode.back().enSymbol != CodeUnit::Symbol::ProgEnd)
		{
			printf("预处理错误：代码意外为空或丢失结束标记！");
			return false;
		}

		return true;
	}

	static void PrintCodeList(const CodeList &listCode)
	{
		FileStream fsStdout(stdout, false);
		for (const auto &it : listCode)
		{
			it.PrintToStream(fsStdout);
		}
	}


	//预处理器报错有待完善

	//预处理器优化有待完善（比如识别经典归0，来回移动，递增递减抵消，无用嵌套抵消等）


	/*
	TODO:预处理后的CodeUnit保存到文件，后缀.bfvc：brainfuck virtual code
	执行启动时进行识别文件类型，bfvc带有文件头（BFVC开头，后跟文件信息）
		对于.bf源代码，进行正常转换并根据具体需要进行优化执行
		.bfvc不进行，也不能进行任何优化，且需要完整检查文件合法性，括号配对等
	支持一定程度的bfvc反编译到bf（嗯你懂的，重复一下code unit，ZRM替换为[-]之类的）
	*/

};