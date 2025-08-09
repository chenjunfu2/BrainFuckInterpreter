# BrainFuckInterpreter
一个简易的BrainFuck解释器</br>
</br>
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
   SELF [-g] ([-f {FileName}]/[-a {Input}]/[-i])
      -g        忽略未知字符，不报错
      -f        从文件中打开并运行BF代码
      -a        从参数中输入并运行BF代码
      -i        从标准流中输入并运行BF代码
      FileName  需要运行的BF代码文件名
      Input     需要运行的BF代码（参数中输入）

</pre>