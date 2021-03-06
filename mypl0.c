/*
* PL/0 complier program (syntax analysis only) implemented in C
*
* The program has been tested on Visual Studio 2010
*
* 使用方法：
* 运行后输入PL/0源程序文件名	 * 运行后输入PL/0源程序文件名
* foutput.txt输出源文件及出错示意（如有错）	 * 回答是否输出虚拟机代码
* 一旦遇到错误就停止语法分析	 * 回答是否输出符号表
* fcode.txt输出虚拟机代码
* foutput.txt输出源文件、出错示意（如有错）和各行对应的生成代码首地址（如无错）
* fresult.txt输出运行结果
* ftable.txt输出符号表
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define bool int
#define true 1
#define false 0

#define norw 21       /* 保留字个数 */
#define txmax 100     /* 符号表容量 */
#define nmax 14       /* 数字的最大位数 */
#define al 10         /* 标识符的最大长度 */
#define maxerr 30     /* 允许的最多错误数 */
#define amax 2048     /* 地址上界*/
#define levmax 3      /* 最大允许过程嵌套声明层数*/
#define cxmax 200     /* 最多的虚拟机代码数 */
#define stacksize 500 /* 运行时数据栈元素最多为500个 */


/* 符号 */
enum symbol {
	nul, ident, number, plus, minus,
	times, slash, oddsym, eql, nequal,
	lss, leq, gtr, geq, lparen,
	rparen, comma, semicolon, period, becomes,
	beginsym, endsym, ifsym, thensym, whilesym,
	writesym, readsym, dosym, callsym, constsym,
	varsym, procsym, lbrace, rbrace, lbracket,
	rbracket, equal, mainsym, elsesym, intsym,
	charsym, selfplus, selfminus, repeatsym, untilsym,
	mod,functionsym,returnsym,
};

#define symnum 48

/* 符号表中的类型 */
enum object {
	integer,
	character,
	func,
};

/* 虚拟机代码指令 */
enum fct {
	lit, opr, lod,
	sto, cal, ini,
	jmp, jpc, sta,
	loa, cpy, chg,
};
#define fctnum 12

/* 虚拟机代码结构 */
struct instruction
{
	enum fct f; /* 虚拟机代码指令 */
	int l;      /* 引用层与声明层的层次差 */
	int a;      /* 根据f的不同而不同 */
};

bool listswitch;   /* 显示虚拟机代码与否 */
bool tableswitch;  /* 显示符号表与否 */
char ch;            /* 存放当前读取的字符，getch 使用 */
enum symbol sym;    /* 当前的符号 */
char id[al + 1];      /* 当前ident，多出的一个字节用于存放0 */
char tem_id[al + 1];	/*存放临时id的副本，以便数组查找使用*/
int num;            /* 当前number */
int cc, ll;         /* getch使用的计数器，cc表示当前字符(ch)的位置，ll为line length缓存区长度 */
int cx;             /* 虚拟机代码指针, 取值范围[0, cxmax-1]*/
char line[81];      /* 读取行缓冲区 */
char a[al + 1];       /* 临时符号，多出的一个字节用于存放0 */
struct instruction code[cxmax]; /* 存放虚拟机代码的数组 */
char word[norw][al];        /* 保留字 */
enum symbol wsym[norw];     /* 保留字对应的符号值 */
enum symbol ssym[256];      /* 单字符的符号值 */
char mnemonic[fctnum][5];   /* 虚拟机代码指令名称 */


							/* 符号表结构 */
struct tablestruct
{
	char name[al];	    /* 名字 */
	enum object kind;	/* 类型：int，char */
	int idx;			/* 如果是数组，存放数组下标 */
	int val;            /* 数值 */
	int level;          /* 所处层 */
	int adr;            /* 地址 */
	int size;           /* 需要分配的数据区空间, 仅procedure使用 */
};

struct tablestruct table[txmax]; /* 符号表 */

FILE* fin;      /* 输入源文件 */
FILE* ftable;	/* 输出符号表 */
FILE* fcode;    /* 输出虚拟机代码 */
FILE* foutput;  /* 输出文件及出错示意（如有错）、各行对应的生成代码首地址（如无错） */
FILE* fresult;  /* 输出执行结果 */
char fname[al]="test.txt";			//------------------------------------调试使用，最后修改-----------------------------------

int is_char_flag = 0;
int is_minus = 0;


void error(int n);
void getsym();
int getch();
void init();
void gen(enum fct x, int y, int z);
void functions(int* ptx, int* pdx);
void function(int* ptx, int* pdx);
void argument(int *ptx, int *pdx);
void declaration_list(int* ptx, int* pdx);
void declaration_stat(int* ptx, int* pdx);
void statement_list(int* ptx);
void statement(int *ptx);
int expression(int *ptx);
int simple_expr(int *ptx);
int additive_expr(int *ptx);
int term(int *ptx);
int factor(int *ptx);
void listcode(int cx0);
void listall();
int position(char* idt, int tx, int idx);
void enter(enum object k, int* ptx, int idx, int* pdx);
void interpret();


/* 主程序开始 */
int main()
{
	printf("Input pl/0 file?   ");
	//scanf("%s", fname);		/* 输入文件名 *///------------------------------------调试使用，最后修改-----------------------------------

	if ((fin = fopen(fname, "r")) == NULL)
	{
		printf("Can't open the input file!\n");
		exit(1);
	}

	ch = fgetc(fin);
	if (ch == EOF)    /* 文件为空 */
	{
		printf("The input file is empty!\n");
		fclose(fin);
		exit(1);
	}
	rewind(fin);

	if ((foutput = fopen("foutput.txt", "w")) == NULL)
	{
		printf("Can't open the output file!\n");
		exit(1);
	}

	if ((ftable = fopen("ftable.txt", "w")) == NULL)
	{
		printf("Can't open ftable.txt file!\n");
		exit(1);
	}

	printf("List object codes?(Y/N)\n");	/* 是否输出虚拟机代码 */
										//scanf("%s", fname);	//--------------------------------调试---------------------------
	fname[0] = 'y';
	listswitch = (fname[0] == 'y' || fname[0] == 'Y');

	printf("List symbol table?(Y/N)\n");	/* 是否输出符号表 */
										//scanf("%s", fname);		//--------------------------------调试---------------------------
	fname[0] = 'y';
	tableswitch = (fname[0] == 'y' || fname[0] == 'Y');

	init();		/* 初始化 */
	cc = ll = cx = 0;
	ch = ' ';

	getsym();
	int tx=0, dx=3;
	int cx0;
	cx0 = cx;
	gen(jmp, 0, 1);		//-----------------
	functions(&tx, &dx);

	if (sym == mainsym)
	{
		code[cx0].a = cx;
		getsym();
		if (sym == lbrace)
		{
			getsym();
			
			declaration_list(&tx, &dx);		/* 处理分程序 */
			statement_list(&tx);		/* 处理分程序 */
			gen(opr, 0, 0);	                    /* 每个过程出口都要使用的释放数据段指令 */
			if (sym != rbrace)
			{
				error(100);	//格式错误，应是右大括号
			}
			else
			{
				printf("\n===Parsing success!===\n");
				fprintf(foutput, "\n===Parsing success!===\n");

				if ((fcode = fopen("fcode.txt", "w")) == NULL)
				{
					printf("Can't open fcode.txt file!\n");
					exit(1);
				}

				if ((fresult = fopen("fresult.txt", "w")) == NULL)
				{
					printf("Can't open fresult.txt file!\n");
					exit(1);
				}

				listall();	 /* 输出所有代码 */
				fclose(fcode);

				interpret();	/* 调用解释执行程序 */
				fclose(fresult);
			}
		}
		else
			error(101);	//格式错误，应是左大括号
	}
	else
		error(102);	//格式错误，应是main


	fclose(foutput);
	fclose(fin);

	//system("pause");

	return 0;
}

/*
* 初始化
*/
void init()
{
	int i;

	/* 设置单字符符号 */
	for (i = 0; i <= 255; i++)
	{
		ssym[i] = nul;
	}
	ssym['+'] = plus;
	ssym['-'] = minus;
	ssym['*'] = times;
	ssym['/'] = slash;
	ssym['('] = lparen;
	ssym[')'] = rparen;
	ssym['='] = eql;
	ssym[','] = comma;
	ssym['.'] = period;
	ssym[';'] = semicolon;
	ssym['{'] = lbrace;
	ssym['}'] = rbrace;
	ssym['['] = lbracket;
	ssym[']'] = rbracket;
	ssym['%'] = mod;



	/* 设置保留字名字,按照字母顺序，便于二分查找 */

	strcpy(&(word[0][0]), "begin");
	strcpy(&(word[1][0]), "call");
	strcpy(&(word[2][0]), "char");
	strcpy(&(word[3][0]), "const");
	strcpy(&(word[4][0]), "do");
	strcpy(&(word[5][0]), "else");
	strcpy(&(word[6][0]), "end");
	strcpy(&(word[7][0]), "function");
	strcpy(&(word[8][0]), "if");
	strcpy(&(word[9][0]), "int");
	strcpy(&(word[10][0]), "main");
	strcpy(&(word[11][0]), "odd");
	strcpy(&(word[12][0]), "procedure");
	strcpy(&(word[13][0]), "read");
	strcpy(&(word[14][0]), "repeat");
	strcpy(&(word[15][0]), "return");
	strcpy(&(word[16][0]), "then");
	strcpy(&(word[17][0]), "until");
	strcpy(&(word[18][0]), "var");
	strcpy(&(word[19][0]), "while");
	strcpy(&(word[20][0]), "write");

	/* 设置保留字符号 */
	wsym[0] = beginsym;
	wsym[1] = callsym;
	wsym[2] = charsym;
	wsym[3] = constsym;
	wsym[4] = dosym;
	wsym[5] = elsesym;
	wsym[6] = endsym;
	wsym[7] = functionsym;
	wsym[8] = ifsym;
	wsym[9] = intsym;
	wsym[10] = mainsym;
	wsym[11] = oddsym;
	wsym[12] = procsym;
	wsym[13] = readsym;
	wsym[14] = repeatsym;
	wsym[15] = returnsym;
	wsym[16] = thensym;
	wsym[17] = untilsym;
	wsym[18] = varsym;
	wsym[19] = whilesym;
	wsym[20] = writesym;

	/* 设置指令名称 */
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[ini][0]), "int");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");
	strcpy(&(mnemonic[sta][0]), "sta");
	strcpy(&(mnemonic[loa][0]), "loa");
	strcpy(&(mnemonic[cpy][0]), "cpy");
	strcpy(&(mnemonic[chg][0]), "chg");
}


/*
*	出错处理，打印出错位置和错误编码
*  遇到错误就退出语法分析
*/
void error(int n)
{
	char space[81];
	memset(space, 32, 81);

	space[cc - 1] = 0; /* 出错时当前符号已经读完，所以cc-1 */

	printf("%s^%d\n", space, n);
	fprintf(foutput, "%s^%d\n", space, n);

	exit(1);
}

/*
* 过滤空格，读取一个字符
* 每次读一行，存入line缓冲区，line被getsym取空后再读一行
* 被函数getsym调用
* 返回值：
*	0 正常返回
*	-1 文件结尾
*/
int getch()
{
	if (cc == ll) /* 判断缓冲区中是否有字符，若无字符，则读入下一行字符到缓冲区中 */
	{
		if (feof(fin))
		{
			/*printf("Program incomplete!\n");
			exit(1);*/
			return -1;
		}
		ll = 0;
		cc = 0;

		ch = ' ';
		while (ch != 10)
		{
			if (EOF == fscanf(fin, "%c", &ch))
			{
				line[ll] = 0;
				break;
			}

			printf("%c", ch);
			fprintf(foutput, "%c", ch);
			line[ll] = ch;
			ll++;
		}
	}
	ch = line[cc];
	cc++;
	return 0;
}

/*
* 词法分析，获取一个符号
*/
void getsym()
{
	int i, j, k;

	while (ch == ' ' || ch == 10 || ch == 9)	/* 过滤空格、换行和制表符 */
	{
		getch();
	}
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) /* 当前的单词是标识符或是保留字 */
	{
		k = 0;
		do {
			if (k < al)
			{
				a[k] = ch;
				k++;
			}
			getch();
		} while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'));
		a[k] = 0;
		strcpy(id, a);
		i = 0;
		j = norw - 1;
		do {    /* 搜索当前单词是否为保留字，使用二分法查找 */
			k = (i + j) / 2;
			if (strcmp(id, word[k]) <= 0)
			{
				j = k - 1;
			}
			if (strcmp(id, word[k]) >= 0)
			{
				i = k + 1;
			}
		} while (i <= j);
		if (i - 1 > j) /* 当前的单词是保留字 */
		{
			sym = wsym[k];
		}
		else /* 当前的单词是标识符 */
		{
			sym = ident;
		}
	}
	else
	{
		if (ch >= '0' && ch <= '9') /* 当前的单词是数字 */
		{
			k = 0;
			num = 0;
			sym = number;
			do {
				num = 10 * num + ch - '0';
				k++;
				getch();
			} while (ch >= '0' && ch <= '9'); /* 获取数字的值 */
			k--;
			if (k > nmax) /* 数字位数太多 */
			{
				error(103);
			}
		}
		else
		{
			if (ch == ':')		/* 检测赋值符号 */
			{
				getch();
				if (ch == '=')
				{
					sym = becomes;
					getch();
				}
				else
				{
					sym = nul;	/* 不能识别的符号 */
				}
			}
			else
			{
				if (ch == '<')		/* 检测小于或小于等于符号 */
				{
					getch();
					if (ch == '=')
					{
						sym = leq;
						getch();
					}
					else
					{
						sym = lss;
					}
				}
				else
				{
					if (ch == '>')		/* 检测大于或大于等于符号 */
					{
						getch();
						if (ch == '=')
						{
							sym = geq;
							getch();
						}
						else
						{
							sym = gtr;
						}
					}
					else
					{
						if (ch == '=')	//检测==符号  或 单独的等于号
						{
							getch();
							if (ch == '=')
							{
								sym = equal;
								getch();
							}
							else
								sym = eql;
						}
						else {
							if (ch == '!')	//检测不等于！=符号
							{
								getch();
								if (ch == '=') {
									sym = nequal;
									getch();
								}
							}
							else {
								if (ch == '+') {	//检测自增运算符++ 或 + 
									getch();
									if (ch == '+')
									{
										sym = selfplus;
										getch();
									}
									else
										sym = plus;
								}
								else
								{
									if (ch == '-') {	//检测自减运算符-- 或 - 
										getch();
										if (ch == '-')
										{
											sym = selfminus;
											getch();
										}
										else
											sym = minus;
									}
									else {
										if (ch == '/') {	//检测注释//或/
											getch();
											if (ch == '/') {	//行注释//,清空行缓存区,重新读取sym
												ll = cc = 0;
												getch();
												getsym();
											}
											else if (ch == '*') {	//区块注释/*，一直读取,直到*/停止
												getch();
												while (1)	/* 过滤空格、换行和制表符 */
												{
													while (ch != '*') {
														getch();
														if (ch == '\0')
															break;
													}
													if (getch() == -1)
														error(104);	//读到文件末尾
													if (ch == '/') {	//读取到区块注释结束符*/
														getch();
														getsym();
														break;
													}
													if (ch == '\0')
														error(105);	//匹配到最后仍没有匹配到*/
												}
											}
											else
												sym = ssym['/'];
										}
										else {
											sym = ssym[ch];		/* 当符号不满足上述条件时，全部按照单字符符号处理 */
											getch();
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

/*
* 生成虚拟机代码
*
* x: instruction.f;
* y: instruction.l;
* z: instruction.a;
*/
void gen(enum fct x, int y, int z)
{
	if (cx >= cxmax)
	{
		printf("Program is too long!\n");	/* 生成的虚拟机代码程序过长 */
		exit(1);
	}
	if (z >= amax)
	{
		printf("Displacement address is too big!\n");	/* 地址偏移越界 */
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	cx++;
}

void functions(int* ptx, int* pdx)
{
	while (sym == functionsym) {
		int i = 3;
		gen(jmp, 0, cx+1);
		function(ptx, &i);
	}
}

void function(int* ptx, int* pdx)
{
	if (sym == functionsym) {
		getsym();
		if (sym == ident) {
			enter(func, ptx, 1, pdx);		//-----------------
			getsym();
			if (sym == lparen) {
				getsym();
				argument(ptx, pdx);
				if (sym == rparen) {
					getsym();
					if (sym == lbrace) {
						getsym();
						declaration_list(ptx, pdx);
						statement_list(ptx);
						if (sym == returnsym) {
							getsym();
							expression(ptx);
							if (sym == semicolon) {
								getsym();
							}
							else
								error(205);
						}

						if (sym == rbrace) {
							gen(opr, 0, 0);
							getsym();
						}
						else
							error(206);
					}
					else
						error(204);
				}
				else
					error(203);	
			}
			else
				error(202);	//函数名后要跟左括号
		}
		else
			error(201);	//没有函数名
	}
	else
		error(200);	//function要以function开头
}

void argument(int *ptx, int *pdx)
{
	int is_int_flag;	//用来记录是int还是char,
	if(sym != intsym && sym!=charsym && sym!=rparen)
		error(199);	//type只能是int或char
	int cnt=0;
	while (sym == intsym || sym == charsym)	//如果是type
	{
		if (sym == intsym)
			is_int_flag = 1;
		else
			is_int_flag = 0;
		getsym();
		if (sym == ident)
		{
			getsym();
			if (is_int_flag == 1)
				enter(integer, ptx, 1, pdx);		/* 填写符号表 -----------------*/
			else
				enter(character, ptx, 1, pdx);
			//gen(sto, 1, cnt++);
			cnt++;
			if (sym == comma )
			{
				getsym();
			}
		}
		else
			error(110);	//格式错误，type后应为ID
	}
	gen(lit, 0, cnt );	//将参数个数lod到栈顶，记录参数个数
	int cx1 = cx;
	for (int i = 0; i < cnt; i++) {
		gen(sto, 1, i + 3);
	}
	int cx2 = cx - 1;
	while (cx2 >= cx1) {
		code[cx2--].l = cnt+4;
	}
	
}


void declaration_list(int* ptx, int* pdx)
{
	//gen(jmp, 0, 1);
	while (sym == intsym || sym == charsym)
	{
		declaration_stat(ptx, pdx);
	}
	gen(ini, 0, *(pdx));

	int i;
	if (tableswitch)		/* 输出符号表 */
	{
		for (i = 1; i <= *(ptx); i++)
		{
			switch (table[i].kind)
			{
			case integer:
				printf("    %d int   %s ", i, table[i].name);
				printf(" idx=%d ", table[i].idx);
				printf(" val=%d ", table[i].val);
				printf(" level=%d ", table[i].level);
				printf(" addr=%d ", table[i].adr);
				printf(" size=%d \n", table[i].size);
				fprintf(ftable, "    %d var   %s ", i, table[i].name);
				fprintf(ftable, "addr=%d\n", table[i].adr);
				break;
			case character:
				printf("    %d char   %s ", i, table[i].name);
				printf(" idx=%d ", table[i].idx);
				printf(" val=%d ", table[i].val);
				printf(" level=%d ", table[i].level);
				printf(" addr=%d ", table[i].adr);
				printf(" size=%d \n", table[i].size);
				fprintf(ftable, "    %d var   %s ", i, table[i].name);
				fprintf(ftable, "addr=%d\n", table[i].adr);
				break;
			case func:
				printf("    %d func   %s ", i, table[i].name);
				printf(" idx=%d ", table[i].idx);
				printf(" val=%d ", table[i].val);
				printf(" level=%d ", table[i].level);
				printf(" addr=%d ", table[i].adr);
				printf(" size=%d \n", table[i].size);
				fprintf(ftable, "    %d var   %s ", i, table[i].name);
				fprintf(ftable, "addr=%d\n", table[i].adr);
				break;
			}
		}
		printf("\n");
		fprintf(ftable, "\n");
	}
}


void declaration_stat(int* ptx, int *pdx)
{
	int is_int_flag;	//用来记录是int还是char,
	if (sym == intsym || sym == charsym)	//如果是type
	{
		if (sym == intsym)
			is_int_flag = 1;
		else
			is_int_flag = 0;
		getsym();
		if (sym == ident)
		{
			getsym();
			if (sym == semicolon)
			{
				if (is_int_flag == 1)
					enter(integer, ptx, 1, pdx);		/* 填写符号表 */
				else
					enter(character, ptx, 1, pdx);
				getsym();
			}
			else if (sym == lbracket)
			{
				getsym();
				if (sym == number)
				{
					if (is_int_flag == 1)
						enter(integer, ptx, num, pdx);	/* 填写符号表 *///----------------------------------------------
					else
						enter(character, ptx, num, pdx);
					getsym();
				}
				else
					error(106);	/* 数组中间应为number */


				if (sym == rbracket)
				{
					getsym();
					if (sym == semicolon)
					{
						getsym();
					}
					else
						error(107);	/* 格式错误，数组声明完应为分号 */
				}
				else
					error(108);	/* 格式错误，应是右中括号 */

			}
			else
				error(109);	/* 格式错误，ID后应为分号或左括号 */
		}
		else
			error(110);	//格式错误，type后应为ID
	}
	else
	{
		error(111);	//type只能是int或char
	}
}



void statement_list(int* ptx)
{
	while (sym == ifsym || sym == whilesym || sym == repeatsym || sym == readsym || sym == writesym || sym == lbrace ||
		sym == lparen || sym == semicolon || sym == ident || sym == number || sym == selfminus || sym == selfplus || sym == minus)
	{
		statement(ptx);
	}
}

void statement(int *ptx)
{
	int tem;
	int cx0;	//记录当前代码序号，回填时使用
	int cx1;	//回填2
	if (sym == ifsym)	//statement 是 if_stat
	{
		getsym();
		if (sym == lparen)
		{
			getsym();
			expression(ptx);

			cx0 = cx;
			gen(jpc, 0, 0);	//条件不满足时跳转，先填充0，后面翻译完statement后进行回填

			if (sym == rparen)
			{
				getsym();
				statement(ptx);
				cx1 = cx;
				gen(jmp, 0, 0);	//if执行完之后跳转到else之外；先填充0，后面翻译完statement后进行回填
				code[cx0].a = cx;	//进行if不满足条件的回填
				if (sym == elsesym)
				{
					getsym();
					statement(ptx);

				}
				code[cx1].a = cx;	//进行if执行完的回填
			}
			else
				error(112);	//expression后应为右括号
		}
		else
			error(113);	//if后应为左括号
	}
	else if (sym == whilesym)	//statement 是 while_stat
	{

		getsym();
		if (sym == lparen)
		{
			cx0 = cx;
			getsym();
			expression(ptx);
			cx1 = cx;

			gen(jpc, 0, 0);	//条件不满足时跳转，先填充0，后面翻译完statement后进行回填

			if (sym == rparen)
			{
				getsym();
				statement(ptx);
			}
			else
				error(114);	//expression后应为右括号

			gen(jmp, 0, cx0);
			code[cx1].a = cx;	//进行if不满足条件的回填
		}
		else
			error(115);	//while后应为左括号
	}
	else if (sym == repeatsym)	//statement 是 repeat_stat---------------------------------------------
	{
		cx0 = cx;

		getsym();
		statement(ptx);
		if (sym == untilsym)
		{
			getsym();
			if (sym == lparen)
			{
				getsym();
				expression(ptx);

				gen(jpc, 0, cx0);	//条件跳转，当不满足条件时,继续跳回循环

				if (sym == rparen)
				{
					getsym();
					if (sym == semicolon) {
						getsym();
					}
					else
						error(136);	//右括号后应该跟分号
				}
				else
				{
					error(116);	//expression后跟右括号
				}
			}
			else
			{
				error(117);	//until后必须跟左括号
			}
		}
		else
			error(118);	//repeat后必须跟until
	}
	else if (sym == readsym)	//statement 是 read_stat
	{
		int i;
		getsym();
		if (sym == ident) {
			i = position(id, *ptx, 0);
			if (i == 0)
			{
				error(119);	//标识符未声明
			}
			getsym();
			int bracket_flag = 0;
			int bracket_i = 0;
			if (sym == lbracket) {	//左中括号，是数组形式
				bracket_flag = 1;
				strcpy(tem_id, id);
				getsym();
				tem = expression(ptx);
				bracket_i = position(tem_id, *ptx, tem);
				if (bracket_i == 0)
					error(126);	/* 标识符未声明 */
				if (sym == rbracket)		//右中括号，数组结束
				{
					getsym();
				}
				else
					error(120);	//数组右边必须是右中括号
			}

			gen(opr, 0, 16);	/* 生成输入指令，读取值到栈顶 */
			if (bracket_flag != 0)
				gen(sta, 0, table[i].adr);
			else
				gen(sto, 0, table[i].adr);/* 将栈顶内容送入变量单元中 */

		}
		else
			error(121);	//read后应为var,而first(var)={ident}

		if (sym == semicolon)
		{
			getsym();
		}
		else
			error(122);	//少了分号
	}
	else if (sym == writesym)	//statement 是 write_stat
	{
		getsym();
		expression(ptx);
		if (is_char_flag)
			gen(opr, 0, 18);	/* 生成输出字符指令，输出栈顶的值 */
		else
			gen(opr, 0, 14);	/* 生成输出数字指令，输出栈顶的值 */
		gen(opr, 0, 15);	/* 生成换行指令 */
		if (sym == semicolon)
		{
			getsym();
		}
		else
			error(123);	//少了分号
	}
	else if (sym == lbrace)	//statement 是 compound_stat
	{
		getsym();
		statement_list(ptx);
		if (sym == rbrace)
		{
			getsym();
		}
		else
			error(124);	//compound_stat的最后应为右大括号

	}
	else if (sym == semicolon)	//statement 是 expression_stat 之二
	{
		getsym();
	}
	else {	//剩下的就是 statement 是 expression_stat 之一，或是其他错误情况
		expression(ptx);
		if (sym == semicolon)
		{
			getsym();
		}
		else
			error(125);	//格式错误，不符合expression_stat格式，错误的statement表达式
	}

}


int expression(int *ptx)
{
	static int equal_flag = 0;	//利用静态变量实现连续赋值
	int i, tem;
	int express_flag = 0;
	int tll = ll;
	int tcc = cc;
	int tsym = sym;
	int tch = ch;
	char tid[al + 1];
	strcpy(tid, id);
	int ans;

	if (sym == ident || sym==minus) {
		is_minus = 0;
		if (sym == minus) {
			is_minus = 1;
			getsym();
		}
		i = position(id, *ptx, 0);
		if (i == 0)
			error(126);	/* 标识符未声明 */
		if (table[i].kind == character)
			is_char_flag = 1;
		else
			is_char_flag = 0;

		getsym();
		if (sym == eql) {
			express_flag = 1;
		}
		if (sym == lbracket) {
			while (sym != rbracket)
				getsym();
			getsym();
			if (sym == eql)
				express_flag = 1;
			//else
			//	error(0);	//左中括号没有右中括号与之结合
		}
	}

	ll = tll;
	cc = tcc;
	sym = tsym;
	ch = tch;
	strcpy(id, tid);

	int bracket_flag = 0;
	int bracket_i = 0;

	if (express_flag) {

		i = position(id, *ptx, 0);
		if (i == 0)
			error(126);	/* 标识符未声明 */

		getsym();	//已经确定前面是ident，所以读入下一个
		if (sym == lbracket) {	//如果ident下一个是左中括号，则进入数组形式
			bracket_flag = 1;
			strcpy(tem_id, id);
			getsym();
			tem = expression(ptx);
			bracket_i = position(tem_id, *ptx, tem);
			if (bracket_i == 0)
				error(126);	/* 标识符未声明 */
			if (equal_flag)		//如果数组左边存在连等，后续lod的时候需要根据当前的栈顶找到要loa的值，因此复制一遍。
				gen(cpy, 0, 0);
			if (sym == rbracket) {
				getsym();
			}
			else
				error(0);	//应有右括号与左括号匹配
		}
		//if(bracket_flag != 0)
		//	gen(lod, 0, table[i].adr);
		if (sym == eql) {
			equal_flag++;
			getsym();
			ans = expression(ptx);
			table[i].val = ans;
		}
		if (bracket_flag != 0) {
			gen(sta, 0, table[i].adr);
		}
		else
			gen(sto, 0, table[i].adr);
		equal_flag--;
		if (equal_flag != 0) {			//如果存在连等，则继续将
			if (bracket_flag != 0) {
				gen(loa, 0, table[i].adr);
			}
			else
				gen(lod, 0, table[i].adr);
		}
	}
	else	//不是var=expression格式
		ans = simple_expr(ptx);
	return ans;
}

int simple_expr(int *ptx)
{
	int i;
	int ans = additive_expr(ptx);

	bool flag[symnum];	//进行符号的临时标记
	for (i = 0; i<symnum; i++)
	{
		flag[i] = false;
	}
	if (sym == gtr || sym == lss || sym == geq || sym == leq || sym == equal || sym == nequal) {
		flag[gtr] = (sym == gtr) ? true : false;
		flag[lss] = (sym == lss) ? true : false;
		flag[geq] = (sym == geq) ? true : false;
		flag[leq] = (sym == leq) ? true : false;
		flag[equal] = (sym == equal) ? true : false;
		flag[nequal] = (sym == nequal) ? true : false;

		getsym();
		additive_expr(ptx);

		if (flag[gtr])
			gen(opr, 0, 12);	/* 生成大于指令 */
		if (flag[lss])
			gen(opr, 0, 10);	/* 生成小于指令 */
		if (flag[geq])
			gen(opr, 0, 11);	/* 生成大于等于指令 */
		if (flag[leq])
			gen(opr, 0, 13);	/* 生成小于等于指令 */
		if (flag[equal])
			gen(opr, 0, 8);		/* 生成判断相等指令 */
		if (flag[nequal])
			gen(opr, 0, 9);		/* 生成判断不等指令 */

		flag[gtr] = false;
		flag[lss] = false;
		flag[geq] = false;
		flag[leq] = false;
		flag[equal] = false;
		flag[nequal] = false;

	}
	return ans;
}

int additive_expr(int *ptx)
{
	int i;
	int ans = term(ptx);	//接收第一个操作数
	int t;	//如果有第二个操作数，接收第二个操作数

	bool flag[symnum];	//进行符号的临时标记
	for (i = 0; i<symnum; i++)
	{
		flag[i] = false;
	}

	while (sym == plus || sym == minus) {
		flag[plus] = (sym == plus) ? true : false;	//进行记录+、或者-号，在读完term后进行生成指令代码
		flag[minus] = (sym == minus) ? true : false;

		getsym();
		t = term(ptx);

		if (flag[plus]) {
			gen(opr, 0, 2);		/* 生成加法指令 */
			ans += t;
		}
		if (flag[minus]) {
			gen(opr, 0, 3);		/* 生成减法指令 */
			ans -= t;
		}

		flag[plus] = 0;		//将标记进行还原
		flag[minus] = 0;
	}
	return ans;
}

int term(int *ptx)
{
	int i;
	int ans = factor(ptx);	//接收第一个操作数
	int t;	//如果有第二个操作数，接收第二个操作数

	bool flag[symnum];	//进行符号的临时标记
	for (i = 0; i<symnum; i++)
	{
		flag[i] = false;
	}
	while (sym == times || sym == slash || sym == mod) {
		flag[times] = (sym == times) ? true : false;
		flag[slash] = (sym == slash) ? true : false;
		flag[mod] = (sym == mod) ? true : false;

		getsym();
		t = factor(ptx);	//接收第二个操作数

		if (flag[times]) {
			gen(opr, 0, 4);		/* 生成乘法指令 */
			ans *= t;
		}
		if (flag[slash]) {
			gen(opr, 0, 5);		/* 生成除法指令 */
			ans /= t;
		}
		if (flag[mod]) {
			gen(opr, 0, 17);	/* 生成模以指令 */
			ans %= t;
		}

		flag[times] = 0;		//还原
		flag[slash] = 0;
		flag[mod] = 0;
	}
	return ans;
}

int factor(int *ptx)
{
	int i;
	int tem;

	bool flag[symnum];	//进行符号的临时标记
	for (i = 0; i<symnum; i++)
	{
		flag[i] = false;
	}

	is_minus = 0;
	if (sym == minus) {
		is_minus = 1;
		getsym();
	}

	if (sym == lparen) {
		getsym();
		tem = expression(ptx);
		if (sym == rparen) {
			getsym();
			return tem;
		}
		else
			error(129);	//expression后应为右括号
	}
	else if (sym == ident ) {
		i = position(id, *ptx, 0);/* 查找标识符在符号表中的位置 */
		if (i == 0)
		{
			error(130);	/* 标识符未声明 */
		}
		getsym();

		if (table[i].kind == func) {		//处理函数部分
			int ti = i;
			if (sym == lparen) {
				getsym();
				int cnt = 3;
				while (sym == ident || sym==number) {
					expression(ptx);
					//gen(sto, 1, cnt++);
					if (sym == comma)
						getsym();
				}
				if (sym == rparen) {
					gen(cal, 1, table[ti].adr);	
					getsym();
					return ;
				}
				else
					error(301);
			}
			else
				error(300);
		}

		int bracket_flag = 0;
		int bracket_i = 0;
		if (sym == lbracket) {	//左中括号，是数组形式
			bracket_flag = 1;
			strcpy(tem_id, id);
			getsym();
			tem = expression(ptx);
			bracket_i = position(tem_id, *ptx, tem);/* 查找数组标识符在符号表中的位置 */
			if (bracket_i == 0)
			{
				error(130);	/* 标识符未声明 */
			}
			if (sym == rbracket)		//右中括号，数组结束
			{
				getsym();
			}
			else
				error(131);	//数组右边必须是右中括号
		}

		if (bracket_flag != 0)
			gen(loa, 0, table[i].adr);	//数组形式的lod--->loa
		else
			gen(lod, 0, table[i].adr);	//普通变量的lod

		if (sym == selfminus || sym == selfplus) {		//a++ 形式		注释讲a++
			if (sym == selfplus) {		//a++,最后栈顶为原来的a			此时栈顶		...a
				
				if (bracket_flag != 0) {
					gen(lit, 0, tem);
					gen(cpy, 0, 0);
					gen(loa, 0, table[i].adr);
				}
				else
					gen(lod, 0, table[i].adr);	//再次将a置于栈顶			执行完栈顶：	...a a
				gen(lit, 0, 1);				//将1置于栈顶				执行完栈顶：	...a a 1
				gen(opr, 0, 2);				//将栈顶前两个数相加			执行完栈顶：	...a a+1
				if (bracket_flag != 0) {
					gen(sta, 0, table[i].adr);
					table[bracket_i].val = table[bracket_i].val + 1;
				}
				else {
					gen(sto, 0, table[i].adr);//将栈顶a+1存入a			执行完栈顶：	...a (将a+1存入a,此时栈顶仍是原来的a)
					table[i].val = table[i].val + 1;
				}
			}
			else {						//a--,同a++,						注释讲a[2]--
				table[i].val = table[i].val - 1;
				if (bracket_flag != 0) {	//a[2]--					此时栈顶		...a[2]
					gen(lit, 0, tem);	//								执行完栈顶	...a[2] 2
					gen(cpy, 0, 0);	//将栈顶复制一下						执行完栈顶：	...a[2] 2 2
					gen(loa, 0, table[i].adr);//						执行完栈顶	...a[2] 2 a[2]
				}
				else
					gen(lod, 0, table[i].adr);
				gen(lit, 0, 1);				//将1置于栈顶				执行完栈顶：	...a[2] 2 a[2] 1
				gen(opr, 0, 3);				//将栈顶前两个数相加			执行完栈顶： ...a[2] 2 a[2]-1
				if (bracket_flag != 0) {
					gen(sta, 0, table[i].adr);	//						执行完栈顶：	...a[2]  (将a[2]-1存放进a[2]的地址了，栈顶元素为a[2])
					table[bracket_i].val = table[bracket_i].val - 1;
				}
				else {
					gen(sto, 0, table[i].adr);//将栈顶a+1存入a,此时栈顶是原来的a
					table[i].val = table[i].val + 1;
				}
			}
			getsym();
		}

		return table[i].val;
	}
	else if (sym == number) {
		if (is_minus)
			num = -num;
		gen(lit, 0, num);	/* 生成立即数指令 */
		getsym();
		return num;
	}
	else if (sym == selfminus || sym == selfplus) {	//++a 形式
		flag[selfminus] = (sym == selfminus) ? true : false;
		flag[selfplus] = (sym == selfplus) ? true : false;
		getsym();
		if (sym == ident) {
			i = position(id, *ptx, 0);/* 查找标识符在符号表中的位置 */
			if (i == 0)
			{
				error(132);	/* 标识符未声明 */
			}
			getsym();

			int bracket_flag = 0;
			int bracket_i = 0;

			if (sym == lbracket) {	//左中括号，是数组形式
				bracket_flag = 1;
				strcpy(tem_id, id);
				getsym();
				tem = expression(ptx);
				bracket_i = position(tem_id, *ptx, tem);/* 查找数组标识符在符号表中的位置 */
				if (bracket_i == 0)
					error(132);	/* 标识符未声明 */

				if (sym == rbracket)		//右中括号，数组结束
				{
					getsym();
				}
				else
					error(133);	//数组右边必须是右中括号
			}

			if (flag[selfplus]) {		//++a,最后的栈顶为a+1						此时栈顶		...
				if (bracket_flag != 0) {
					gen(cpy, 0, 0);
					gen(cpy, 0, 0);
					gen(loa, 0, table[i].adr);
				}
				else {
					gen(lod, 0, table[i].adr);//								执行完栈顶：	...a
				}
				gen(lit, 0, 1);				//原来栈顶为a,将1放入栈顶				执行完栈顶：	...a 1
				gen(opr, 0, 2);				//将a和1相加，得到栈顶a+1				执行完栈顶：	...a+1
				if (bracket_flag != 0) {
					table[bracket_i].val = table[bracket_i].val + 1;
					gen(sta, 0, table[i].adr);
					gen(loa, 0, table[i].adr);
				}
				else {
					table[i].val = table[i].val + 1;
					gen(sto, 0, table[i].adr);	//将栈顶a+1存入a					执行完栈顶：	...		(将a+1存入a)
					gen(lod, 0, table[i].adr);	//再将a置于栈顶，此时的a是a+1		执行完栈顶：	...a+1	(此时在lod a,得到的是a+1的值)
				}
				flag[selfplus] = false;	//清除标记


			}
			if (flag[selfminus]) {		//--a，同上			注释解释--a[3];		此时栈顶		...3
				if (bracket_flag != 0) {
					gen(cpy, 0, 0);			//复制栈顶							执行完栈顶：	...3 3
					gen(cpy, 0, 0);			//复制栈顶							执行完栈顶：	...3 3 3
					gen(loa, 0, table[i].adr);//由3数组a[3]的值					执行完栈顶：	...3 3 a[3]
				}
				else {
					gen(lod, 0, table[i].adr);
				}
				gen(lit, 0, 1);				//将1放入栈顶						执行完栈顶：	...3 3 a[3] 1
				gen(opr, 0, 3);				//将a和1相减，得到栈顶a-1				执行完栈顶：	...3 3 a[3]-1
				if (bracket_flag != 0) {
					table[bracket_i].val = table[bracket_i].val - 1;
					gen(sta, 0, table[i].adr);	//将栈顶存入a[3]					执行完栈顶：	...3   (此时a[3]的值已经是a[3]-1)
					gen(loa, 0, table[i].adr);	//由3数组a[3]的值				执行完栈顶：	...a[3]	(此时的栈顶a[3]已经是a[3]-1了)
				}
				else {
					table[i].val = table[i].val - 1;
					gen(sto, 0, table[i].adr);
					gen(lod, 0, table[i].adr);
				}
				flag[selfminus] = false;
			}

		}
		else
			error(134);	//++、--后应为var
	}
	else
		error(135);	//factor元素为三种
}




/*
* 在符号表中加入一项
*
* k:      标识符的种类为int，char
* ptx:    符号表尾指针的指针，为了可以改变符号表尾指针的值
* size:	符号表元素大小，若为0代表变量，若大于0则表示数组
*
*/
void enter(enum object k, int* ptx, int idx, int *pdx)
{
	int i;
	for (i = 0; i < idx; i++) {
		(*ptx)++;
		strcpy(table[(*ptx)].name, id); /* 符号表的name域记录标识符的名字 */
		table[(*ptx)].kind = k;
		table[(*ptx)].idx = i;
		table[(*ptx)].val = -1;	//将初始值全部初始化为-1，防止初始化为0时对于read的变量当作除数产生错误。
		switch (k)
		{
		case integer:	/* 变量 */
			table[(*ptx)].adr = (*pdx);
			(*pdx)++;
			break;
		case character:	/* 变量 */
			table[(*ptx)].adr = (*pdx);
			(*pdx)++;
			break;
		case func:
			table[(*ptx)].adr = cx;
			break;
		}
	}
}

/*
* 查找标识符在符号表中的位置，从tx开始倒序查找标识符
* 找到则返回在符号表中的位置，否则返回0
*
* id:    要查找的名字
* tx:     当前符号表尾指针
*/
int position(char* id, int tx, int idx)
{
	int i;
	strcpy(table[0].name, id);
	table[0].idx = idx;
	i = tx;
	while (strcmp(table[i].name, id) != 0 || table[i].idx != idx)
	{
		i--;
	}
	return i;
}

/*
* 输出目标代码清单
*/
void listcode(int cx0)
{
	int i;
	if (listswitch)
	{
		printf("\n");
		for (i = cx0; i < cx; i++)
		{
			printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
		}
	}
}

/*
* 输出所有目标代码
*/
void listall()
{
	int i;
	if (listswitch)
	{
		for (i = 0; i < cx; i++)
		{
			printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
			fprintf(fcode, "%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
		}
	}
}

/*
* 解释程序
*/
void interpret()
{
	int p = 0; /* 指令指针 */
	int b = 1; /* 指令基址 */
	int t = 0; /* 栈顶指针 */
	struct instruction i;	/* 存放当前指令 */
	int s[stacksize];	/* 栈 */

	printf("Start pl0\n");
	fprintf(fresult, "Start pl0\n");
	int arg=0;	//函数参数个数
	s[0] = 0; /* s[0]不用 */
	s[1] = 0; /* 主程序的三个联系单元均置为0 */
	s[2] = 0;
	s[3] = 0;
	do {
		i = code[p];	/* 读当前指令 */
		p = p + 1;
		switch (i.f)
		{
		case lit:	/* 将常量a的值取到栈顶 */
			t = t + 1;
			s[t] = i.a;
			break;
		case opr:	/* 数学、逻辑运算 */
			switch (i.a)
			{
			case 0:  /* 函数调用结束后返回 */
				s[b-arg] = s[t];		//将返回结果保存到清理当前过程后的栈顶
				t = b - 1 - arg;
				p = s[t + arg + 3];
				b = s[t + arg + 2];
				t = t + 1;
				break;
			case 1: /* 栈顶元素取反 */
				s[t] = -s[t];
				break;
			case 2: /* 次栈顶项加上栈顶项，退两个栈元素，相加值进栈 */
				t = t - 1;
				s[t] = s[t] + s[t + 1];
				break;
			case 3:/* 次栈顶项减去栈顶项 */
				t = t - 1;
				s[t] = s[t] - s[t + 1];
				break;
			case 4:/* 次栈顶项乘以栈顶项 */
				t = t - 1;
				s[t] = s[t] * s[t + 1];
				break;
			case 5:/* 次栈顶项除以栈顶项 */
				t = t - 1;
				s[t] = s[t] / s[t + 1];
				break;
			case 6:/* 栈顶元素的奇偶判断 */
				s[t] = s[t] % 2;
				break;
			case 8:/* 次栈顶项与栈顶项是否相等 */
				t = t - 1;
				s[t] = (s[t] == s[t + 1]);
				break;
			case 9:/* 次栈顶项与栈顶项是否不等 */
				t = t - 1;
				s[t] = (s[t] != s[t + 1]);
				break;
			case 10:/* 次栈顶项是否小于栈顶项 */
				t = t - 1;
				s[t] = (s[t] < s[t + 1]);
				break;
			case 11:/* 次栈顶项是否大于等于栈顶项 */
				t = t - 1;
				s[t] = (s[t] >= s[t + 1]);
				break;
			case 12:/* 次栈顶项是否大于栈顶项 */
				t = t - 1;
				s[t] = (s[t] > s[t + 1]);
				break;
			case 13: /* 次栈顶项是否小于等于栈顶项 */
				t = t - 1;
				s[t] = (s[t] <= s[t + 1]);
				break;
			case 14:/* 栈顶值输出 */
				printf("%d", s[t]);
				fprintf(fresult, "%d", s[t]);
				t = t - 1;
				break;
			case 15:/* 输出换行符 */
				printf("\n");
				fprintf(fresult, "\n");
				break;
			case 16:/* 读入一个输入置于栈顶 */
				t = t + 1;
				printf("?");
				fprintf(fresult, "?");
				scanf("%d", &(s[t]));
				fprintf(fresult, "%d\n", s[t]);
				break;
			case 17:/* 次栈顶项模以栈顶项 */
				t = t - 1;
				s[t] = s[t] % s[t + 1];
				break;
			case 18:/* 栈顶字符值输出 */
				printf("%c", s[t]);
				fprintf(fresult, "%c", s[t]);
				t = t - 1;
				break;
			}
			break;
		case lod:	/* 取相对当前过程的数据基地址为a的内存的值到栈顶 */
			t = t + 1;
			s[t] = s[base(i.l, s, b) + i.a];
			break;
		case loa:	/* 将栈顶变为地址为a,偏移为栈顶的值   执行前栈顶：...3   执行后栈顶：...a[3]  即从3得到a[3]的值  */
			s[t] = s[1 + s[t] + i.a];
			break;
		case sto:	/* 栈顶的值存到相对当前过程的数据基地址为a的内存 */
			if (i.l == 0) {
				s[base(i.l, s, b) + i.a] = s[t];
				t = t - 1;
			}
			else {
				int tem = t + 1 + i.a;
				s[tem] = s[tem - i.l];
				arg = i.l-3;
				//t--;
			}
			break;
		case sta:	/* 栈顶的值存入地址为次栈顶的内存		执行前栈顶：...3 999   执行后栈顶：...  将999存入a[3]   */
			s[1 + s[t - 1] + i.a] = s[t];
			t = t - 2;
			break;
		case cal:	/* 调用子过程 */
			s[t + 2] = base(i.l, s, b);	/* 将父过程基地址入栈，即建立静态链 */
			s[t + 3] = b;	/* 将本过程基地址入栈，即建立动态链 */
			s[t + 4] = p;	/* 将当前指令指针入栈，即保存返回地址 */
			b = t + 2;	/* 改变基地址指针值为新过程的基地址 -----------------*/
			p = i.a;	/* 跳转 */
			break;
		case ini:	/* 在数据栈中为被调用的过程开辟a个单元的数据区 */
			t = t + i.a;
			break;
		case jmp:	/* 直接跳转 */
			p = i.a;
			break;
		case jpc:	/* 条件跳转 */
			if (s[t] == 0)
				p = i.a;
			t = t - 1;
			break;
		case cpy:	/* 将栈顶复制一下，放入栈顶 */
			s[t + 1] = s[t];
			t = t + 1;
			break;
		case chg:	/* 将次栈顶和次次栈顶交换 */
			s[t + 1] = s[t - 1];	//利用s[t+1]作为临时变量交换次栈顶s[t-1]和次次栈顶s[t-2]
			s[t - 1] = s[t - 2];
			s[t - 2] = s[t + 1];
			break;
		}
	} while (p != 0);
	printf("End pl0\n");
	fprintf(fresult, "End pl0\n");
}

int base(int l, int* s, int b)
{
	int b1;
	b1 = b;
	while (l > 0)
	{
		b1 = s[b1];
		l--;
	}
	return b1;
}