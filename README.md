# BrainFuckInterpreter
动态BrainFuck解释器</br>
这个项目是BF动态解释器，使用C++编写</br>
如果你需要的是BF直接映射到C++语言的转换器（某种程度上算编译器？），可以前往我的另一个项目：</br>
[https://github.com/chenjunfu2/BrainFuck](https://github.com/chenjunfu2/BrainFuck)</br>

# 用法介绍
使用命令行启动程序，执行完毕后自动退出</br>
<pre>
命令行说明：
   - SELF指代程序自身名字
   - 方括号[]内为可选项
   - 圆括号()内为必选项
   - 花括号{}内为参数名
   - 多个选项用/隔开
   - 以-开头的选项大小写敏感
   - 选项开头符号-与/等价

命令行选项：
   SELF [-g/-o/-s] ([-f {FileName}]/[-a {Input}]/[-i])
      -g        忽略未知字符，不报错
      -o        对代码进行优化
      -s        输出优化后的代码
      -f        从文件中打开并运行BF代码
      -a        从参数中输入并运行BF代码
      -i        从标准流中输入并运行BF代码
      FileName  需要运行的BF代码文件名
      Input     需要运行的BF代码（参数中输入记得加双引号）
</pre>
使用示例：</br>
<img width="1285" height="382" alt="使用示例" src="https://github.com/user-attachments/assets/e58b311e-47ec-429e-b4d4-c60aff4671ef" />
