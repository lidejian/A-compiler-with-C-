/*
* PL/0 complier program (syntax analysis only) implemented in C
*
* The program has been tested on Visual Studio 2010
*
* 使用方法：
* 运行后输入PL/0源程序文件名
* foutput.txt输出源文件及出错示意（如有错）
* 一旦遇到错误就停止语法分析
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define bool int
#define true 1
#define false 0

#define norw 19       /* 保留字个数 */
#define txmax 100     /* 符号表容量 */
#define nmax 14       /* 数字的最大位数 */
#define al 10         /* 标识符的最大长度 */


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
	charsym, selfplus,selfminus,repeatsym,untilsym,mod
};

#define symnum 42

/* 符号表中的类型 */
enum object {
	integer,
	character,
};


char ch;            /* 存放当前读取的字符，getch 使用 */
enum symbol sym;    /* 当前的符号 */
char id[al + 1];      /* 当前ident，多出的一个字节用于存放0 */
int num;            /* 当前number */
int cc, ll;         /* getch使用的计数器，cc表示当前字符(ch)的位置，ll为line length缓存区长度 */
char line[81];      /* 读取行缓冲区 */
char a[al + 1];       /* 临时符号，多出的一个字节用于存放0 */
char word[norw][al];        /* 保留字 */
enum symbol wsym[norw];     /* 保留字对应的符号值 */
enum symbol ssym[256];      /* 单字符的符号值 */


							/* 符号表结构 */
struct tablestruct
{
	char name[al];	    /* 名字 */
	enum object kind;	/* 类型：int，char */
	int size;			/* 如果是数组，存放数组大小 */
};

struct tablestruct table[txmax]; /* 符号表 */

FILE* fin;      /* 输入源文件 */
FILE* foutput;  /* 输出文件及出错示意（如有错） */
char fname[al] = "test.txt";			//------------------------------------调试使用，最后修改-----------------------------------


void error(int n);
void getsym();
int getch();
void init();
int declaration_list(int tx);
void declaration_stat(int* ptx);
void statement_list(int* ptx);
void statement(int *ptx);
void expression(int *ptx);
void simple_expr(int *ptx);
void additive_expr(int *ptx);
void term(int *ptx);
void factor(int *ptx);
int position(char* idt, int tx);
void enter(enum object k, int* ptx, int s);


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

	init();		/* 初始化 */
	cc = ll = 0;
	ch = ' ';

	getsym();

	if (sym == mainsym)
	{
		getsym();
		if (sym == lbrace)
		{
			getsym();
			int i = declaration_list(0);		/* 处理分程序 */
			statement_list(&i);		/* 处理分程序 */
			if (sym != rbrace)
			{
				error(92);	//格式错误，应是右大括号
			}
			else
			{
				printf("\n===Parsing success!===\n");
				fprintf(foutput, "\n===Parsing success!===\n");
			}
		}
		else
			error(91);	//格式错误，应是左大括号
	}
	else
		error(90);	//格式错误，应是main


	fclose(foutput);
	fclose(fin);

	system("pause");

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
	strcpy(&(word[7][0]), "if");
	strcpy(&(word[8][0]), "int");
	strcpy(&(word[9][0]), "main");
	strcpy(&(word[10][0]), "odd");
	strcpy(&(word[11][0]), "procedure");
	strcpy(&(word[12][0]), "read");
	strcpy(&(word[13][0]), "repeat");
	strcpy(&(word[14][0]), "then");
	strcpy(&(word[15][0]), "until");
	strcpy(&(word[16][0]), "var");
	strcpy(&(word[17][0]), "while");
	strcpy(&(word[18][0]), "write");

	/* 设置保留字符号 */
	wsym[0] = beginsym;
	wsym[1] = callsym;
	wsym[2] = charsym;
	wsym[3] = constsym;
	wsym[4] = dosym;
	wsym[5] = elsesym;
	wsym[6] = endsym;
	wsym[7] = ifsym;
	wsym[8] = intsym;
	wsym[9] = mainsym;
	wsym[10] = oddsym;
	wsym[11] = procsym;
	wsym[12] = readsym;
	wsym[13] = repeatsym;
	wsym[14] = thensym;
	wsym[15] = untilsym;
	wsym[16] = varsym;
	wsym[17] = whilesym;
	wsym[18] = writesym;	
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
*	1 文件结尾
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
				error(30);
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
														error(0);	//读到文件末尾
													if (ch == '/') {	//读取到区块注释结束符*/
														getch();
														getsym();
														break;
													}
													if (ch == '\0')
														error(0);	//匹配到最后仍没有匹配到*/
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



int declaration_list(int tx)
{
	while (sym == intsym || sym == charsym)
	{
		declaration_stat(&tx);
	}
	return tx;
}


void declaration_stat(int* ptx)
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
					enter(integer, ptx, 0);		/* 填写符号表 */
				else
					enter(character, ptx, 0);
				getsym();
			}
			else if (sym == lbracket)
			{
				getsym();
				if (sym == number)
				{
					if (is_int_flag == 1)
						enter(integer, ptx, num);	/* 填写符号表 *///----------------------------------------------
					else
						enter(character, ptx, num);
					getsym();
				}
				else
					error(0);	/* 数组中间应为number */


				if (sym == rbracket)
				{
					getsym();
					if (sym == semicolon)
					{
						getsym();
					}
					else
						error(0);	/* 格式错误，数组声明完应为分号 */
				}
				else
					error(93);	/* 格式错误，应是右中括号 */

			}
			else
				error(0);	/* 格式错误，ID后应为分号或左括号 */
		}
		else
			error(0);	//格式错误，type后应为ID
	}
	else
	{
		error(0);	//type只能是int或char
	}
}



void statement_list(int* ptx)
{
	while (sym == ifsym || sym == whilesym || sym == repeatsym || sym == readsym || sym == writesym ||
		sym == lparen || sym == semicolon || sym == ident || sym == number)
	{
		statement(ptx);
	}
}

void statement(int *ptx)
{
	if (sym == ifsym)	//statement 是 if_stat
	{
		getsym();
		if (sym == lparen)
		{
			getsym();
			expression(ptx);
			if (sym == rparen)
			{
				getsym();
				statement(ptx);
				if (sym == elsesym)
				{
					getsym();
					statement(ptx);
				}
			}
			else
				error(0);	//expression后应为右括号
		}
		else
			error(0);	//if后应为左括号
	}
	else if (sym == whilesym)	//statement 是 while_stat
	{
		getsym();
		if (sym == lparen)
		{
			getsym();
			expression(ptx);
			if (sym == rparen)
			{
				getsym();
				statement(ptx);
			}
			else
				error(0);	//expression后应为右括号
		}
		else
			error(0);	//while后应为左括号
	}
	else if (sym == repeatsym)	//statement 是 repeat_stat---------------------------------------------
	{
		getsym();
		statement(ptx);
		if (sym == untilsym)
		{
			getsym();
			if (sym == lparen) 
			{
				getsym();
				expression(ptx);
				if (sym == rparen)
				{
					getsym();
				}
				else
				{
					error(0);	//expression后跟右括号
				}
			}
			else
			{
				error(0);	//until后必须跟左括号
			}
		}
		else
			error(0);	//repeat后必须跟until
	}
	else if (sym == readsym)	//statement 是 read_stat
	{
		int i;
		getsym();
		if (sym == ident) {
			i = position(id, *ptx);
			if (i == 0)
			{
				error(0);	//标识符未声明
			}
			getsym();
			if (sym == lbracket) {	//左中括号，是数组形式
				getsym();
				expression(ptx);
				if (sym == rbracket)		//右中括号，数组结束
				{
					getsym();
				}
				else
					error(0);	//数组右边必须是右中括号
			}
		}
		else
			error(0);	//read后应为var,而first(var)={ident}

		if (sym == semicolon)
		{
			getsym();
		}
		else
			error(0);	//少了分号
	}
	else if (sym == writesym)	//statement 是 write_stat
	{
		getsym();
		expression(ptx);
		if (sym == semicolon)
		{
			getsym();
		}
		else
			error(0);	//少了分号
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
			error(0);	//compound_stat的最后应为右大括号

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
			error(0);	//格式错误，不符合expression_stat格式，错误的statement表达式
	}

}


void expression(int *ptx)
{
	int i;
	if (sym == selfminus || sym == selfplus)	//++a形式
		getsym();
	if (sym == lparen || sym == number) {
		simple_expr(ptx);
	}
	else if (sym == ident) {
		i = position(id, *ptx);/* 查找标识符在符号表中的位置 */
		if (i == 0)
		{
			error(0);	/* 标识符未声明 */
		}
		getsym();
		if (sym == lbracket) {	//左中括号，是数组形式
			getsym();
			expression(ptx);
			if (sym == rbracket)		//右中括号，数组结束
			{
				getsym();
			}
			else
				error(0);	//数组右边必须是右中括号
		}

		if (sym == eql) {	//expression之一
			getsym();
			expression(ptx);
		}
		if(sym == selfplus || sym == selfminus) {		//expression 扩展： expression: var++ | var--
			getsym();
		}

		if (sym == times || sym == slash || sym == mod || sym == plus || sym == minus ) {
			do {
				getsym();
				term(ptx);
				while (sym == times || sym == slash || sym == mod) {
					getsym();
					factor(ptx);
				}
			} while (sym == plus || sym == minus);
		}
	}
	else {
		error(0);	//first（expression）只能是ident、lparen、number
	}
	
	if (sym == gtr || sym == lss || sym == geq || sym == leq || sym == equal || sym == nequal) {
		getsym();
		additive_expr(ptx);
	}
}

void simple_expr(int *ptx)
{
	additive_expr(ptx);
	if (sym == gtr || sym == lss || sym == geq || sym == leq || sym == equal || sym == nequal) {
		getsym();
		additive_expr(ptx);
	}
}

void additive_expr(int *ptx)
{
	term(ptx);
	while (sym == plus || sym == minus ) {
		getsym();
		term(ptx);
	}
}

void term(int *ptx)
{
	factor(ptx);
	while (sym == times || sym == slash || sym == mod) {
		getsym();
		factor(ptx);
	}
}

void factor(int *ptx)
{
	int i;
	if (sym == lparen) {
		getsym();
		expression(ptx);
		if (sym == rparen) {
			getsym();
		}
		else
			error(0);	//expression后应为右括号
	}
	else if (sym == ident) {
		i = position(id, *ptx);/* 查找标识符在符号表中的位置 */
		if (i == 0)
		{
			error(0);	/* 标识符未声明 */
		}
		getsym();
		if (sym == lbracket) {	//左中括号，是数组形式
			getsym();
			expression(ptx);
			if (sym == rbracket)		//右中括号，数组结束
			{
				getsym();
			}
			else
				error(0);	//数组右边必须是右中括号
		}
		
		if (sym == selfminus || sym == selfplus) {		//a++ 形式
			getsym();
		}
	}
	else if (sym == number) {
		getsym();
	}
	else if (sym == selfminus || sym == selfplus) {	//++a 形式
		getsym();
		if (sym == ident) {
			i = position(id, *ptx);/* 查找标识符在符号表中的位置 */
			if (i == 0)
			{
				error(0);	/* 标识符未声明 */
			}
			getsym();
			if (sym == lbracket) {	//左中括号，是数组形式
				getsym();
				expression(ptx);
				if (sym == rbracket)		//右中括号，数组结束
				{
					getsym();
				}
				else
					error(0);	//数组右边必须是右中括号
			}
		}
		else
			error(0);	//++、--后应为var
	}
	else
		error(0);	//factor元素为三种
}




/*
* 在符号表中加入一项
*
* k:      标识符的种类为int，char
* ptx:    符号表尾指针的指针，为了可以改变符号表尾指针的值
* size:	符号表元素大小，若为0代表变量，若大于0则表示数组
*
*/
void enter(enum object k, int* ptx, int s)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, id); /* 符号表的name域记录标识符的名字 */
	table[(*ptx)].kind = k;
	table[(*ptx)].size = s;
}

/*
* 查找标识符在符号表中的位置，从tx开始倒序查找标识符
* 找到则返回在符号表中的位置，否则返回0
*
* id:    要查找的名字
* tx:     当前符号表尾指针
*/
int position(char* id, int tx)
{
	int i;
	strcpy(table[0].name, id);
	i = tx;
	while (strcmp(table[i].name, id) != 0)
	{
		i--;
	}
	return i;
}