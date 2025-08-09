#pragma once

#include "CodeUnit.hpp"
#include "FileStream.hpp"
#include "CharStream.hpp"

#include <cstdio>
#include <vector>
#include <cctype>

template<typename StreamType>
class Preprocessor//预处理器
{
private:
	//注意szDupCount需要自行初始化！！！本函数仅递增
	static size_t FindDuplicate(StreamType &sStream, const char cFind)
	{
		size_t szDupCount = 0;
		while(true)
		{
			char cRead = sStream.GetChar();
			if (sStream.Eof())
			{
				break;//文件结束，离开
			}

			//判断是否相同（处理重复字符）
			if (cRead != cFind)
			{
				sStream.UnGet();//回退1读取
				break;//遇到不同，直接离开
			}
			
			++szDupCount;//相同继续合并
		}

		return szDupCount;
	}
public:
	//如果该函数返回false，那么任何使用listCode进行执行的操作都是未定义行为，当然，读取失败现场是没问题的
	static bool PreprocessInFile(StreamType &sStream, CodeList &listCode, bool bIgnoreUnknownChar = false)
	{
		if (!sStream)//文件是NULL，返回
		{
			return false;
		}

		listCode.clear();//清理以便读取

		//块语句栈（存储索引）
		std::vector<size_t> codeBlockStack;

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

			CodeUnit CurCode{};
			switch (cRead)
			{
			case '>':
				{
					CurCode.enSymbol = CodeUnit::NextMov;
					CurCode.szMovOffset = 1;
					//查找重复值
					size_t szDupCount = FindDuplicate(sStream, cRead);
					CurCode.szMovOffset += szDupCount;
					szColumn += szDupCount;//连续读取处理列号
				}
				break;
			case '<':
				{
					CurCode.enSymbol = CodeUnit::PrevMov;
					CurCode.szMovOffset = 1;
					//查找重复值
					size_t szDupCount = FindDuplicate(sStream, cRead);
					CurCode.szMovOffset += szDupCount;
					szColumn += szDupCount;//连续读取处理列号
				}
				break;
			case '+':
				{
					CurCode.enSymbol = CodeUnit::AddCur;
					CurCode.szCalcValue = 1;
					//查找重复值
					size_t szDupCount = FindDuplicate(sStream, cRead);
					CurCode.szCalcValue += szDupCount;
					szColumn += szDupCount;//连续读取处理列号
				}
				break;
			case '-':
				{
					CurCode.enSymbol = CodeUnit::SubCur;
					CurCode.szCalcValue = 1;
					//查找重复值
					size_t szDupCount = FindDuplicate(sStream, cRead);
					CurCode.szCalcValue += szDupCount;
					szColumn += szDupCount;//连续读取处理列号
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

					//获取栈顶索引
					size_t szStackTop = codeBlockStack.size() - 1;

					CurCode.enSymbol = CodeUnit::LoopEnd;//找到一个loopend，寻找最近的配对
					CurCode.szJmpIndex = codeBlockStack[szStackTop];//获取栈顶，也就是最近的一个loopbeg，存入跳转位置，让当前能跳转到loopbeg
					listCode.push_back(CurCode);//提前插入，开始处理重复循环块

					//现在，读取所有重复的]，让前面与之配对的所有loopbeg都跳转到重复的最后，而对每个重复的]处理正确配对
					//这样，减少重复loopend的来回跳转开销与判断开销
					size_t szDupCount = FindDuplicate(sStream, cRead);

					//在此之前，先判断一下szStackTop够不够szDupCount，不够说明括号未配对，报错
					if (szDupCount > szStackTop)
					{
						printf("解析失败[line:%zu,column:%zu]：括号未匹配\n", szLine, szColumn + szStackTop + 1);//通过剩余未匹配括号来定位错误点
						return false;
					}

					szColumn += szDupCount;//处理列号

					//获取szDupCount后，往尾部塞入多个循环块，并把最后一个循环块作为前面所有判断的跳转目标
					while (szDupCount-- > 0)//循环szDupCount次
					{
						CurCode.enSymbol = CodeUnit::LoopEnd;
						CurCode.szJmpIndex = codeBlockStack[--szStackTop];//取出更上一个的loopbeg进行配对
						listCode.push_back(CurCode);//插入
					}

					//到这里，loopend跳转目标全部匹配完成
					//但是loopbeg还未匹配，获取最后一个位置后循环写入
					size_t szJmpIndex = listCode.size();//因为所有loopend都插入了，当前尾后index就是最后一个位置

					//设置前面所有的loopbeg的跳转位置为此位置之后
					for (size_t szStackIndex = szStackTop; szStackIndex != codeBlockStack.size(); ++szStackIndex)
					{
						size_t szListIndex = codeBlockStack[szStackIndex];//获取loopbeg在list中的idx
						listCode[szListIndex].szJmpIndex = szJmpIndex;//设置目标位置
					}

					//至此，艺术已成！
					//给栈里面的缓存全弹出，这里把szStackTop作为size而非index，于是元素减少为szStackTop个
					codeBlockStack.resize(szStackTop);
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
		listCode.push_back(CodeUnit{ .enSymbol = CodeUnit::ProgEnd });
		
		return true;
	}
	//预处理器报错有待完善

};