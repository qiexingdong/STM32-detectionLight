///=====================================================================
//文件名称： NumToStr.h文件
//制作单位：SD-Arm(sumcu.suda.edu.cn)
//更新记录：
//【20200916】刘鑫，实现多种数据转换为字符串并返回的功能
//【20200918】陈浪，整理和规范NumToStr.h和NumToStr.c文件
//移植规则：【固定】
//=====================================================================
#include "NumToStr.h"
char str[MAX_SIZE];//存储转化后的字符串


//======================================================================
//函数名称：return_char
//功能概要：将转化后的字符存入字符数组
//参数说明：data:数据：存入的一字节数据    
//函数返回：无
//======================================================================
void return_char(char data) {
	int i = 0;
	i = strlen(str);
	str[i] = data;
}
//======================================================================
//函数名称：NumStr_putc
//功能概要：给数组返回一个字符
//参数说明：c:数据：经过处理后的一字节数据的ASCII值
//函数返回：无
//======================================================================
static void NumStr_putc(int c, int *count, NUM_STR *info)
{
	
	
	switch (info->dest) {
	case DEST_CONSOLE:
		info->func((char)c);
		break;
	case DEST_STRING:
		*(info->loc) = (unsigned char)c;
		++(info->loc);
		break;
	default:
		break;
	}
	*count += 1;
}
//======================================================================
//函数名称：NumStr_mkfloatstr
//功能概要：将浮点数转化为字符串
//参数说明： *f:浮点型数据：float型数据
//       c[]:字符串数组：转化后的到的字符串数组
//函数返回：无
//======================================================================
int NumStr_mkfloatstr(double* f, char c[], int precision_width)
{
                uint32_t p,i,j,k,a[5]={0},b[19]={0};
	uint64_t q;
	double tempf;
	int zeroNum=precision_width;   //【20201119】-1/3
	p=0;
	q=0;
	//float m;
	double m;
	p=(uint32_t)*f;     //整数部分
	tempf=(*f-p);
	if(precision_width==0)
	               q=(*f-p)*100000000000000000;    //小数部分
	else
	{
		while(precision_width--)    
		tempf*=10;
		q=tempf;
	}
	m=*f-p;
	k=0;
	while(m<1&&m!=0)    //【20201119】-2/3   原为while(m<1)
	{
		m=m*10;
		k++;
	};
	k--;          //记录小数点后到第一位不为零的小数之前 有多少个0.

		i=0;
	if(0==p)//整数部分为0
		c[i++]='0';
	else//整数部分不为0
	{
		j=0;
		while(p!=0)//倒序的数字
		{
			a[j++]=p%10;
			p/=10;
		}
		while(j>0)//将倒序数字转为正序，并转换为对应的字符形式
		{
			c[i++]=a[j-1]+'0';
			--j;
		}
	}
	c[i++]='.';   //小数点
	if(0==q)     //小数部分为0
		while(zeroNum--) c[i++]='0';     //【20201119】-3/3   原为c[i++] = '0';
	else    //小数部分不为0
	{
		j=0;

		while(q!=0)//倒序的数字
		{
			b[j++]=q%10;
			q/=10;
		}
		while(k!=0)
		{
			b[j++]=0;
			k--;
		}
		while(j>0)//将倒序数字转为正序，并转换为对应的字符形式
		{
			c[i++]=b[j-1]+'0';
			--j;
		}
	}
	c[i]='\0';
	return i;
              
}
//======================================================================
//函数名称：NumStr_mknumstr
//功能概要：将整形数据转化为字符串保存在numstr中
//参数说明：numstr:字符串首地址：用于接收转化为字符串的整型数据
//       nump:无符号整形数据：将转化为整形的数据传入函数
//       neg:有无符号:判断数据时有符号还是无符号整形
//       radix:格式选择:确定输出格式例如：十进制或十六进制...
//函数返回：无
//======================================================================
static int NumStr_mknumstr(char *numstr, void *nump, int neg, int radix)
{
	int a, b, c;
	unsigned int ua, ub, uc;

	int nlen;
	char *nstrp;

	nlen = 0;
	nstrp = numstr;
	*nstrp++ = '\0';

	if (neg) {
		a = *(int *) nump;
		if (a == 0) {
			*nstrp = '0';
			++nlen;
			goto done;
		}
		while (a != 0) {
			b = (int) a / (int) radix;
			c = (int) a - ((int) b * (int) radix);
			if (c < 0) {
				c = ~c + 1 + '0';
			} else {
				c = c + '0';
			}
			a = b;
			*nstrp++ = (char) c;
			++nlen;
		}
	} else {
		ua = *(unsigned int *) nump;
		if (ua == 0) {
			*nstrp = '0';
			++nlen;
			goto done;
		}
		while (ua != 0) {
			ub = (unsigned int) ua / (unsigned int) radix;
			uc = (unsigned int) ua - ((unsigned int) ub * (unsigned int) radix);
			if (uc < 10) {
				uc = uc + '0';
			} else {
				uc = uc - 10 + 'A';
			}
			ua = ub;
			*nstrp++ = (char) uc;
			++nlen;
		}
	}
	done: return nlen;
}
/********************************************************************/
static void NumStr_pad_zero(int curlen, int field_width, int *count,
		NUM_STR *info) {
	int i;

	for (i = curlen; i < field_width; i++) {
		NumStr_putc('0', count, info);
	}
}

/********************************************************************/
static void NumStr_pad_space(int curlen, int field_width, int *count,
		NUM_STR *info) {
	int i;

	for (i = curlen; i < field_width; i++) {
		NumStr_putc(' ', count, info);
	}
}
//======================================================================
//函数名称：NumStr
//功能概要：根据接收的参数信息对数据进行转换输出
//参数说明：info:传递字符串的信息
//       fmt:输出格式信息
//       ap:va_list指针指向格式信息输出
//函数返回：无
//======================================================================
int NumStr(NUM_STR *info, const char *fmt, char * ap)
{
//	#pragma anon_unions
	/*union ftooc
	{
		double a1;
		struct{
			int c1;
			int c2;
		};
		struct{
			char b1;
			char b2;
			char b3;
			char b4;
			char b5;
			char b6;
			char b7;
			char b8;
		}tempc;

	}un1;*/
	char *p;
	int c;
	//char temp1;
	char vstr[33]={'0'};
	char *vstrp;
	int vlen;

	int done;
	int count;

	int flags_used;
	int field_width;
	int length_modifier;
	int precision_used;
	int precision_width;
    //int test=0;
	int ival;                  //用来接收整形数据
	double fval;                //用来接收浮点数据
	int schar, dschar;
	int *ivalp;
	char *sval;
	int cval;
	unsigned int uval;

	count = 0;
	length_modifier=0;
	precision_used=TRUE;
	//从格式字符串第一个字符开始格式化输出数据
	for (p = (char *) fmt; (c = *p) != 0; p++)
	{
		//读取%前面的字符，直接输出的字符和最后的\n类的转义字符
		if (c != '%')
		{
#ifndef UNIX_DEBUG
			//%之前的字符
			if (c != '\n') {
				NumStr_putc(c, &count, info);
			}
			//输出换行符
			else {
				NumStr_putc(0x0D /* CR */, &count, info);
				NumStr_putc(0x0A /* LF */, &count, info);
			}
#else
			NumStr_putc(c, &count, info);
#endif

			continue;
		}


		//此时字符为%  查看百分号后的第一个格式字符，对输出格式字符并做出相应输出判断
		//检查相应的修改标志
		flags_used = 0;
		done = FALSE;
		while (!done) {
			switch (/* c = */*++p) {
			case '-':
				flags_used |= FLAGS_MINUS;
				break;
			case '+':
				flags_used |= FLAGS_PLUS;
				break;
			case ' ':
				flags_used |= FLAGS_SPACE;
				break;
			case '0':
				flags_used |= FLAGS_ZERO;
				break;
			case '#':
				flags_used |= FLAGS_POUND;
				break;
			default:
				/* we've gone one char too far */
				--p;
				done = TRUE;
				break;
			}
		}


		//检查%后再下一个字符 确定输出长度
		field_width = 0;
		done = FALSE;
		while (!done) {
			switch (c = *++p) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				field_width = (field_width * 10) + (c - '0');
				break;
			default:
				/* we've gone one char too far */
				--p;
				done = TRUE;
				break;
			}
		}
		//此时字符为%看是否有.输入
		if (/* (c = *++p) */*++p == '.') {
			/* precision_used = TRUE; */

			/*
			 * Must get precision field width, if present.
			 */
			precision_width = 0;
			done = FALSE;
			while (!done) {
				switch ( c = *++p) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
#if 1
				precision_width = (precision_width * 10) +	(c - '0');

#endif
					break;
				default:
					/* we've gone one char too far */
					--p;
					done = TRUE;
					break;
				}
			}
		} else {
			/* we've gone one char too far */
			--p;
#if 1
			precision_used = FALSE;
			precision_width = 0;
#endif
		}
		//判断 输出的是否有lf、LF等
		switch (/* c = */*++p) {
		case 'h':
			// length_modifier |= LENMOD_h;
			break;
		case 'l':
			 length_modifier |= LENMOD_l;
			break;
		case 'L':
			 length_modifier |= LENMOD_L;
			break;
		default:
			/* we've gone one char too far */
			--p;
			break;
		}

		//p为格式字符串的首地址
		switch (c = *++p) {
		case 'd':
		case 'i':
			 //此时ap为指向可变参数列表的第一个参数
			ival = (int) va_arg(ap, int);  //返回当前参数，并将ap指向下一个参数
			vlen = NumStr_mknumstr(vstr, &ival, TRUE, 10);//将整型转换成字符型保存在vstr中，返回字符数vlen
			vstrp = &vstr[vlen];  //指向转换后的最后一个字符

			//若参数小于0，添加'-'号，并将字符长度加1
			if (ival < 0) {
				schar = '-';
				++vlen;
			}
			//若参数不小于0，添加‘+’号或者‘ ’或者为0，并将字符长度加1
			else {
				if (IS_FLAG_PLUS(flags_used)) {
					schar = '+';
					++vlen;
				} else {
					if (IS_FLAG_SPACE(flags_used)) {
						schar = ' ';
						++vlen;
					} else {
						schar = 0;
					}
				}
			}
			dschar = FALSE;

			/*
			 * do the ZERO pad.
			 */
			if (IS_FLAG_ZERO(flags_used)) {
				if (schar)
					NumStr_putc(schar, &count, info);  //输出符号
				dschar = TRUE;
				NumStr_pad_zero(vlen, field_width, &count, info);
				vlen = field_width;
			} else {
				if (!IS_FLAG_MINUS(flags_used)) {
					NumStr_pad_space(vlen, field_width, &count, info);  //输出空格

					if (schar)
						NumStr_putc(schar, &count, info);
					dschar = TRUE;
				}
			}

			/* the string was built in reverse order, now display in */
			/* correct order */
			if (!dschar && schar) {
				NumStr_putc(schar, &count, info);
			}
			goto cont_xd;  //开始跳转

		//------------------此处添加浮点数输出功能---------------------------------------
		case 'f':
			//浮点型数据被当做 double型处理，组合方式如下：
			//3.140001 对应浮点数:0x0x4048F5C3；
			//3.140001 对double数:0x40091EB85F577E6C
			//实际传入变参数 函数后 浮点数变为:0x6000000040091EB8 实际是将其转化为double型数据
			//但是顺序反了，我们要做的是将后四字节提前，然后把0x60移到紧接着前四个字节后的位置。
			//【20200816】  XSX，解决浮点数不优化问题
            
           
			ap = ap +4; //返回字符串方法中，此项优化会导致出错
			fval = (double)va_arg(ap, double);
		
			//【20200816】  XSX，解决负数的浮点数输出问题
			//若参数小于0，添加'-'号，并将字符长度加1
			if (fval < 0) {
				schar = '-';
				fval = -fval;				//负数取它的正数
				++vlen;
			}
			//若参数不小于0，添加‘+’号或者‘ ’或者为0，并将字符长度加1
			else {
				if (IS_FLAG_PLUS(flags_used)) {
					schar = '+';
					++vlen;
				}
				else {
					if (IS_FLAG_SPACE(flags_used)) {
						schar = ' ';
						++vlen;
					}
					else {
						schar = 0;
					}
				}
			}
			
			dschar = FALSE;

			//确定是否使用小数点精度数值
			if (precision_used)
			{
				vlen = NumStr_mkfloatstr(&fval, vstr, precision_width);//将浮点型转换成字符型保存在vstr中，返回字符数vlen
				vstrp = &vstr[0];  //指向转换后的第一个字符
			}
			else
			{
				if (length_modifier == 0)   //浮点型精度到小数点后6位
				{
					vlen = NumStr_mkfloatstr(&fval, vstr, 6);//将浮点型转换成字符型保存在vstr中，返回字符数vlen
					vstrp = &vstr[0];  //指向转换后的第一个字符
				}
				else
				{
					vlen = NumStr_mkfloatstr(&fval, vstr, 6);//将浮点型转换成字符型保存在vstr中，返回字符数vlen
					vstrp = &vstr[0];  //指向转换后的第一个字符
				}

			}

			/*
			 * do the ZERO pad.
			 */
			if (IS_FLAG_ZERO(flags_used)) {
				if (schar)
					NumStr_putc(schar, &count, info);  //输出符号
				dschar = TRUE;
				NumStr_pad_zero(vlen, field_width, &count, info);
				vlen = field_width;
			}
			else {
				if (!IS_FLAG_MINUS(flags_used)) {
					NumStr_pad_space(vlen, field_width, &count, info);  //输出空格

					if (schar)
						NumStr_putc(schar, &count, info);
					dschar = TRUE;
				}
			}

			/* the string was built in reverse order, now display in */
			/* correct order */
			if (!dschar && schar) {
				NumStr_putc(schar, &count, info);
			}
			while (*vstrp)
				NumStr_putc(*vstrp++, &count, info);
			if (IS_FLAG_MINUS(flags_used)) {
				NumStr_pad_space(vlen, field_width, &count, info);
			}
			break;
        //以下是16进制整形输出
		case 'x':
		case 'X':
			uval = (unsigned int) va_arg(ap, unsigned int);
			vlen = NumStr_mknumstr(vstr, &uval, FALSE, 16);
			vstrp = &vstr[vlen];

			dschar = FALSE;
			if (IS_FLAG_ZERO(flags_used)) {
				if (IS_FLAG_POUND(flags_used)) {
					NumStr_putc('0', &count, info);
					NumStr_putc('x', &count, info);
					/*vlen += 2;*/
					dschar = TRUE;
				}
				NumStr_pad_zero(vlen, field_width, &count, info);
				vlen = field_width;
			} else {
				if (!IS_FLAG_MINUS(flags_used)) {
					if (IS_FLAG_POUND(flags_used)) {
						vlen += 2;
					}
					NumStr_pad_space(vlen, field_width, &count, info);
					if (IS_FLAG_POUND(flags_used)) {
						NumStr_putc('0', &count, info);
						NumStr_putc('x', &count, info);
						dschar = TRUE;
					}
				}
			}

			if ((IS_FLAG_POUND(flags_used)) && !dschar) {
				NumStr_putc('0', &count, info);
				NumStr_putc('x', &count, info);
				vlen += 2;
			}
			goto cont_xd;
		//无符号以八进制表示的整数输出
		case 'o':
			uval = (unsigned int) va_arg(ap, unsigned int);
			vlen = NumStr_mknumstr(vstr, &uval, FALSE, 8);
			goto cont_u;
		//二进制输出
		case 'b':
			uval = (unsigned int) va_arg(ap, unsigned int);
			vlen = NumStr_mknumstr(vstr, &uval, FALSE, 2);
			goto cont_u;
		//输出指针的值
		case 'p':
			uval = (unsigned int) va_arg(ap, void *);
			vlen = NumStr_mknumstr(vstr, &uval, FALSE, 16);
			goto cont_u;
		case 'u':
			uval = (unsigned int) va_arg(ap, unsigned int);
			vlen = NumStr_mknumstr(vstr, &uval, FALSE, 10);

			cont_u: vstrp = &vstr[vlen];

			if (IS_FLAG_ZERO(flags_used)) {
				NumStr_pad_zero(vlen, field_width, &count, info);
				vlen = field_width;
			} else {
				if (!IS_FLAG_MINUS(flags_used)) {
					NumStr_pad_space(vlen, field_width, &count, info);
				}
			}

			cont_xd: while (*vstrp)
				NumStr_putc(*vstrp--, &count, info);

			if (IS_FLAG_MINUS(flags_used)) {
				NumStr_pad_space(vlen, field_width, &count, info);
			}

			break;
        //字符型输出
		case 'c':
			cval = (char) va_arg(ap, unsigned int);
			NumStr_putc(cval, &count, info);
			break;
		case 's':
			sval = (char *) va_arg(ap, char *);
			if (sval) {
				vlen = strlen(sval);
				if (!IS_FLAG_MINUS(flags_used)) {
					NumStr_pad_space(vlen, field_width, &count, info);
				}
				if(precision_width!=0)
				{
					while (*sval)
					{
						if(precision_width>=1)
						{
							precision_width--;
							NumStr_putc(*sval++, &count, info);
						}
						else {
							break;
						}
					}
				}
				else    //precision_width=0情况
				{
					while (*sval)
						NumStr_putc(*sval++, &count, info);
				}
				if (IS_FLAG_MINUS(flags_used)) {
					NumStr_pad_space(vlen, field_width, &count, info);
				}
			}
			break;
		case 'n':
			ivalp = (int *) va_arg(ap, int *);
			*ivalp = count;
			break;

		default:
			NumStr_putc(c, &count, info);
			break;
		}
	}
	return count;
}

//======================================================================
//函数名称：NumToStr
//功能概要：根据输入的参数进行解析然后将数据转换成字符串返回
//参数说明：const char *:打印参数字符串：对输入的字符串进行解析并按格式输出
//例如： value=NumToStr("c=%c\n",c); value得到的NumToStr(char *)型的返回值，
//       存储参数转换成字符串后的首地址
//函数返回：字符串首地址
//======================================================================
char *NumToStr (const char *fmt, ...)
{
	char * ap;
	NUM_STR info;
	memset(str, 0, sizeof(str));//清空str字符数组
	info.dest = DEST_CONSOLE;  //宏定义为1
	info.func = &return_char;     //调用的串口输出函数
	va_start(ap, fmt);  //对ap 进行初始化，让它指向可变参数表里面的第一个参数
	NumStr(&info, fmt, ap);
	va_end(ap);
	return str;
}

