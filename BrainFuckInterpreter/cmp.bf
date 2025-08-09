#读入两个数，输出左边比右边的大小：
#beg

	#-------------construct char-------------#
	#cur = [0]
	>>>>>>>>				#goto [8]
	++++[>++++++++<-]		#set [9] = 4*8 = 32(ASCII:' ')
	>>++++++++++			#set [10] = 10(ASCII:'\n')
	<<<<<<<<<<				#goto [0]
	#-------------read left char to 0 and 6-------------#
	#cur = [0]
	,.						#read char to [0] and feedback
	[->>>>>>+>+<<<<<<<]		#move [0] to [6] and [7]
	>>>>>>>					#goto [7]
	[-<<<<<<<+>>>>>>>]		#move [7] to [0]
	#-------------print space-------------#
	#cur = [7]
	>>.						#print [0]
	<<<<<<<<				#goto [1]
	#-------------read right char to 1 and 7-------------#
	#cur = [1]
	,.						#read char to [1] and feedback
	[->>>>>>+>+<<<<<<<]		#move [1] to [7] and [8]
	>>>>>>>					#goto [8]
	[-<<<<<<<+>>>>>>>]		#move [8] to [1]
	#-------------print line and left char space-------------#
	#cur = [8]
	>>.						#print [10]
	<<<<.					#print [6]
	>>>.					#print [9]
	<<<<<<<					#goto [2]
	#-------------cmp-------------#
	#cur = [2]
	[-]+	#[2]=1
	>>[-]	#[4]=0
	<<		#goto [2]
	[		#while([2]!=0)
		>[-]+	#[3]=1
		<<<		#goto [0]
		[		#while([0]!=0)
			[->>>>+<<<<]	#move [0] to [4]
			>>>[-]			#[3]=0
			<<<				#goto [0]
		]		#while([0]!=0)
		>>>		#goto [3]
		[		#while([3]!=0)
			<[-]	#[2]=0
			>[-]	#[3]=0
		]		#while([3]!=0)
		>-[-<<<<+>>>>]		#--[4] and move [4] to [0]
		<[-]+	#[3]=1
		<<		#goto [1]
		[			#while([1]!=0)
			[->>>+<<<]		#move [1] to [4]
			>>[-]			#[3]=0
			<<				#goto [1]
		]			#while([1]!=0)
		>>		#goto [3]
		[		#while([3]!=0)
			<[-]	#[2]=0
			>[-]	#[3]=0
		]		#while([3]!=0)
		>-[-<<<+>>>]		#--[4] and move [4] to [1]
		<<		#goto [2]
	]		#while([2]!=0)
	+		#[2]=1
	<<+		#++[0]
	[		#while([0]!=0)
		[-]>>[-]		#[0]=[2]=0
		#now at [2] do something and goto [0]
		>++++++++[>++++++++<-]>--.<<		#write '>'
		<<		#goto [0]
	]		#while([0]!=0)
	>>		#goto [2]
	[		#while([2]!=0)
		[-]+	#[2]=1
		<+		#++[1]
		[		#while([1]!=0)
			[-]>[-]		#[1]=[2]=0
			#now at [2] do something and goto [1]
			>++++++++[>++++++++<-]>----.<<		#write '<'
			<		#goto [1]
		]		#while([1]!=0)
		>		#goto [2]
		[		#while([2]!=0)
			#now at [2] do something and goto [2]
			>++++++++[>++++++++<-]>---.<<		#write '='
			[-]		#[2]=0
		]		#while([2]!=0)
	]		#while([2]!=0)
	#-------------print space and right char line-------------#
	#cur = [2]
	>>>>>>>.		#print [9]
	<<.				#print [7]
	>>>.			#print [10]

#end