# BrainFuckInterpreter
一个简易的BrainFuck解释器</br>
命令行说明：</br>
   \- SELF指代程序自身名字</br>
   \- 方括号[]内为可选项</br>
   \- 圆括号()内为必选项</br>
   \- 花括号{}内为参数名</br>
   \- 多个选项用/隔开</br>
   \- 以-开头的选项大小写敏感</br>
   \- 选项开头符号-与/等价</br>
</br>
命令行选项：</br>
   SELF [-g] ([-f {FileName}]/[-a {Input}]/[-i])</br>
      -g        忽略未知字符，不报错</br>
      -f        从文件中打开并运行BF代码</br>
      -a        从参数中输入并运行BF代码</br>
      -i        从标准流中输入并运行BF代码</br>
      FileName  需要运行的BF代码文件名</br>
      Input     需要运行的BF代码（参数中输入）</br>