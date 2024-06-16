姓名：强陶  学号：522031910206

UART配置：波特率115200，数据位8位，无校验位，停止位1位，收发数据类型ASCII

串口通信协议指令：

1.?：返回帮助菜单
格式: ?@
作用：返回帮助菜单
返回格式：帮助菜单

2.INIT TIME：初始化
格式：INIT TIME@
作用：(1)初始化日期为24-06-16 (2)初始化时间为12-00-00 (3)初始化时钟为11-59-59
返回格式：Init Complete!

3. SET TIME：设置时间
格式：SET TIME xx:xx:xx@  其中第一项为小时，第二项为分钟，第三项为秒
作用：将时间设置为xx:xx:xx
返回格式：SETTIME:xx:xx:xx

4. SET DATE：设置日期
格式：SET DATE xx:xx:xx@ 其中第一项为年，第二项为月，第三项为日
作用：将日期设置为xx:xx:xx
返回格式：SETDATE:xx-xx-xx

5. SET ALARM：设置闹钟
格式：SET ALARM xx:xx:xx@  其中第一项为小时，第二项为分钟，第三项为秒
作用：将闹钟设在xx:xx:xx 当闹钟时间到时，会输出"It's the time!! Wake up!!"并播放音乐
返回格式：SETALARM:xx:xx:xx

6. GET TIME：获取时间
格式：GET TIME@(支持大小写混用与空格容错)
作用：返回当前时间
返回格式：TIME:xx:xx:xx，其中第一项为小时，第二项为分钟，第三项为秒

7. GET DATE：获取日期
格式：GET DATE@
作用：返回当前日期
返回格式：DATE:xx:xx:xx，其中第一项为年，第二项为月，第三项为日

8. GET ALARM：获取闹钟时间
格式：GET ALARM@
作用：返回当前闹钟时间
返回格式：ALARM:xx:xx:xx，其中第一项为小时，第二项为分钟，第三项为秒

9. CACULATE: 两位数加减运算
格式:xx+xx@ 或者 xx-xx@ (如果涉及到个位数，需要在前方补零)
作用：进行两位数加减运算
返回格式: ANSWER:xx+xx=xx(xx-xx=xx) 其中xx是运算的结果 


注：
1. 重新load以后需要进行INIT CLOCK@操作

2. 每条指令后必须加上@作结尾

3. 每条指令的格式固定，不可随意增加空格

4. 对于GET TIME命令，不区分大小写，且具有空格容错

5. 对于错误指令，会输出"Invalid Inputs"