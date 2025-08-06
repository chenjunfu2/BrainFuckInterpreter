#pragma once

#include "CodeUnit.hpp"
#include "FileStream.hpp"

#include <cstdio>
#include <vector>
#include <cctype>


class Preprocessor//预处理器
{
private:
	//注意szDupCount需要自行初始化！！！本函数仅递增
	static void FindDuplicate(FileStream &sFile, const char cFind, size_t& szDupCount)
	{
		while(true)
		{
			char cRead;
			sFile.Read(cRead);
			if (sFile.Eof())
			{
				return;//返回上级处理
			}

			//判断是否相同（处理重复字符）
			if (cRead != cFind)
			{
				sFile.MovFilePos(-1);//回退1读取
				return;//遇到不同，直接返回
			}
			
			++szDupCount;//相同继续合并
		}
	}
public:
	//如果该函数返回false，那么任何使用listCode进行执行的操作都是未定义行为，当然，读取失败现场是没问题的
	static bool PreprocessInFile(FileStream &sFile, CodeList &listCode)
	{
		if (!sFile)//文件是NULL，返回
		{
			return false;
		}

		listCode.clear();//清理以便读取

		//块语句栈（存储索引）
		std::vector<size_t> codeBlockStack;

		//读取并转换到代码列表
		while (true)
		{
			char cRead;
			sFile.Read(cRead);
			if (sFile.Eof())
			{
				break;//读完离开
			}

			CodeUnit CurCode{};
			switch (cRead)
			{
			case '>':
				{
					CurCode.enSymbol = CodeUnit::NextMov;
					CurCode.szMovOffset = 1;
					//查找重复值
					FindDuplicate(sFile, cRead, CurCode.szMovOffset);
				}
				break;
			case '<':
				{
					CurCode.enSymbol = CodeUnit::PrevMov;
					CurCode.szMovOffset = 1;
					//查找重复值
					FindDuplicate(sFile, cRead, CurCode.szMovOffset);
				}
				break;
			case '+':
				{
					CurCode.enSymbol = CodeUnit::AddCur;
					CurCode.szCalcValue = 1;
					//查找重复值
					FindDuplicate(sFile, cRead, CurCode.szCalcValue);
				}
				break;
			case '-':
				{
					CurCode.enSymbol = CodeUnit::SubCur;
					CurCode.szCalcValue = 1;
					//查找重复值
					FindDuplicate(sFile, cRead, CurCode.szCalcValue);
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
						printf("解析失败：括号未匹配\n");
						return false;
					}

					//获取栈顶索引
					size_t szStackTop = codeBlockStack.size() - 1;

					CurCode.enSymbol = CodeUnit::LoopEnd;//找到一个loopend，寻找最近的配对
					CurCode.szJmpIndex = codeBlockStack[szStackTop];//获取栈顶，也就是最近的一个loopbeg，存入跳转位置，让当前能跳转到loopbeg
					listCode.push_back(CurCode);//提前插入，开始处理重复循环块

					//现在，读取所有重复的]，让前面与之配对的所有loopbeg都跳转到重复的最后，而对每个重复的]处理正确配对
					//这样，减少重复loopend的来回跳转开销与判断开销
					size_t szDupCount = 0;
					FindDuplicate(sFile, cRead, szDupCount);

					//在此之前，先判断一下szStackTop够不够szDupCount，不够说明括号未配对，报错
					if (szDupCount > szStackTop)
					{
						printf("解析失败：括号未匹配\n");
						return false;
					}

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
					CurCode.enSymbol = CodeUnit::DbgInfo;
				}
				break;
			case '#':
				{
					//处理注释
					//读取，直到换行符
					bool bCR = false;//\r
					bool bLF = false;//\n
					while (true)
					{
						char cRead;
						sFile.Read(cRead);
						if (sFile.Eof())
						{
							break;//离开循环，回到外层判断
						}

						if (cRead == '\r')
						{
							bCR = true;
						}
						else if (cRead == '\n')
						{
							bLF = true;
						}
						else if (bCR || bLF)
						{
							sFile.MovFilePos(-1);//回退1字节，退出循环
							break;
						}
					}

				}
				continue;//注意此处为continue而非break，不走下面默认压入
			default:
				if (isspace(cRead))
				{
					continue;//跳过空白，直接继续
				}
				//其余未知字符且不在注释内，报错
				
				//todo报错：行号字符数
				return false;//注意报错直接返回
			}

			//压入列表
			listCode.push_back(CurCode);
		}

		//判断栈内是否还有残留的括号，是，则代表括号未配对，报错
		if (!codeBlockStack.empty())
		{
			printf("解析失败：括号未匹配\n");
			return false;
		}

		//完成，尾部插入一个卫兵字符，也就是ProgEnd，执行到代表程序结束
		listCode.push_back(CodeUnit{ .enSymbol = CodeUnit::ProgEnd });
		
		return true;
	}
	//预处理器报错有待完善

};