///=====================================================================
//文件名称： NumToStr.h文件
//制作单位：SD-Arm(sumcu.suda.edu.cn)
//更新记录：
//【20200916】刘鑫，实现多种数据转换为字符串并返回的功能
//【20200918】陈浪，整理和规范NumToStr.h和NumToStr.c文件
//移植规则：【固定】
//=====================================================================
#ifndef	NUMTOSTR_H//防止重复定义（PRINT_H 开头）
#define NUMTOSTR_H

#include "string.h"
#include "mcu.h"

#define va_start _crt_va_start
#define va_arg _crt_va_arg
#define va_end _crt_va_end

//typedef char *  va_list;
//以下函数用于对不确定参数数量的函数进行参数指针定位
//将n的长度化为int长度的整数倍
#define _INTSIZEOF(n)   ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
//初始定位变参数函数指针位置
#define _crt_va_start(ap,v) ( ap = (char *)&(v) + _INTSIZEOF(v) )
//依次读取变参数的数值
#define _crt_va_arg(ap,t)   ( *(t *)((ap+=_INTSIZEOF(t)),ap-_INTSIZEOF(t)) )
//放空ap指针为下一次调用做准备
#define _crt_va_end(ap)     ( ap = (char *)0 )

//用于传递字符串参数的信息结构体
typedef struct {
	int dest;                         //确定信息结构体的类型：默认类型或字符串类型
	void (*func)(char);       //链接输出功能函数
	char *loc;                        //若在字符串模式用于存储字符尾地址
} NUM_STR;


//======================================================================
//函数名称：NumToStr
//功能概要：根据输入的参数进行解析然后将数据转换成字符串返回
//参数说明：const char *:打印参数字符串：对输入的字符串进行解析并按格式输出
//例如： value=NumToStr("c=%c\n",c); value得到的NumToStr(char *)型的返回值，
//       存储参数转换成字符串后的首地址
//函数返回：字符串首地址
//======================================================================
char *NumToStr(const char *, ...);//声明函数NumToStr

#define MAX_SIZE 200 //字符串返回值最大长度

#define DEST_CONSOLE    (1)
#define DEST_STRING     (2)
//用于分辨%f与%lf的宏定义字符
#define LENMOD_l  1
#define LENMOD_L  2
//用于判断%后输出格式的宏定义
#define FLAGS_MINUS     (0x01)
#define FLAGS_PLUS      (0x02)
#define FLAGS_SPACE     (0x04)
#define FLAGS_ZERO      (0x08)
#define FLAGS_POUND     (0x10)

#define IS_FLAG_MINUS(a)    (a & FLAGS_MINUS)
#define IS_FLAG_PLUS(a)     (a & FLAGS_PLUS)
#define IS_FLAG_SPACE(a)    (a & FLAGS_SPACE)
#define IS_FLAG_ZERO(a)     (a & FLAGS_ZERO)
#define IS_FLAG_POUND(a)    (a & FLAGS_POUND)


#ifdef	FALSE
#undef	FALSE
#endif
#define FALSE	(0)

#ifdef	TRUE
#undef	TRUE
#endif
#define	TRUE	(1)

#endif  //防止重复定义（NUMTOSTR_H 结尾）


//======================================================================
/*说明
 *
 *NumToStr函数原型： char *NumToStr(const char *, ...);
printf函数原型：   int printf(const char *, ...);

NumToStr函数是通过printf改造后的一个函数，与printf相比，在实现的功能和返回值类型上有所区别
1. 实现的功能：printf为格式化输出语句，可以将可变长参数中的各种变量按照指定格式打印
               NumToStr可以将可变长参数中的各种变量按照指定格式转换成字符串返回
2. 返回值类型：printf：int 型，返回的是要打印的字符个数，
               NumToStr：char * 型，返回的是要打印的字符串的首地址
使用方法：//定义一个字符型指针变量value
          char *value;
          //将想要装换成字符串的内容作为参数传入NumToStr中，使用value变量存储字符串的首地址
          value=NumToStr("a=%lf\n",a);
          //此时value存储的就是字符串内容，可以把value当成字符串格式的变量进行使用

 
C.1 NumToStr调用的一般格式
NumToStr函数调用的一般形式为：NumToStr(“格式控制字符串”, 输出表列)。
其中格式控制字符串用于指定输出格式。格式控制串可由格式字符串和非格式字符串两种组成。格式字符串是以%开头
的字符串，在%后面跟有各种格式字符，以说明输出数据的类型、形式、长度、小数位数等。如：
   ” %d”表示按十进制整型输出；
   ” %ld”表示按十进制长整型输出；
   ” %f”表示浮点型输出。
   ” %lf”表示double型输出。
   ” %c”表示按字符型输出。
   ” \n”表示换行符等。
       非格式字符串原样输出，在显示中起提示作用。输出表列中给出了各个输出项，要求格式字符串和各输出项在数量
和类型上应该一一对应。
C.1.1 格式字符串
       格式字符串的一般形式为：[标志][输出最小宽度][精度][长度]类型，其中方括号[]中的项为可选项。
以下说明各项的意义。
1. 类型
类型字符用以表示输出数据的类型，其格式符和意义如表C-1所示。
表C-1 类型字符
格式字符	                             意义
  d	             以十进制形式输出带符号整数(正数不输出符号)
  o	             以八进制形式输出无符号整数(不输出前缀0)
 x,X	   以十六进制形式输出无符号整数(不输出前缀Ox)
  u	             以十进制形式输出无符号整数
  f	             以小数形式输出单、双精度实数
 e,E	   以指数形式输出单、双精度实数
 g,G	   以%f或%e中较短的输出宽度输出单、双精度实数
  c	             输出单个字符
  s	             输出字符串
2. 标志
标志字符为 -、+、# 和空格四种，其意义如表C-2所示。
标志	                           意义
 -	            结果左对齐，右边填空格
 +	            输出符号(正号或负号)
3. 输出最小宽度
        用十进制整数来表示输出的最少位数。若实际位数多于定义的宽度，则按实际位数输出，若实际位数少于定义
的宽度则补以空格或0。
4. 精度
        精度格式符以“.”开头，后跟十进制整数。本项的意义是：如果输出数字，则表示小数的位数；如果输出的
是字符，则表示输出字符的个数；若实际位数大于所定义的精度数，则截去超过的部分。
5. 长度
长度格式符为h、l两种，h表示按短整型量输出，l表示按长整型量输出。

 *
 */



