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

#define norw 13       /* 保留字个数 */
#define txmax 100     /* 符号表容量 */
#define nmax 14       /* 数字的最大位数 */
#define al 10         /* 标识符的最大长度 */


/* 符号 */
enum symbol {
    nul,         ident,     number,     plus,      minus, 
    times,       slash,     oddsym,     eql,       neq,//neq删除，用nequal代替 
    lss,         leq,       gtr,        geq,       lparen, 
    rparen,      comma,     semicolon,  period,    becomes, 
    beginsym,    endsym,    ifsym,      thensym,   whilesym, 
    writesym,    readsym,   dosym,      callsym,   constsym, 
    varsym,      procsym,   
};

enum symbol {
	main,		lparen,		rparen,		lbrace,		rbrace,
	lbracket,	rbracket,	equal,		nequal
}

#define symnum 32

/* 符号表中的类型 */
enum object {
	int,
	char,
};


char ch;            /* 存放当前读取的字符，getch 使用 */
enum symbol sym;    /* 当前的符号 */
char id[al+1];      /* 当前ident，多出的一个字节用于存放0 */
int num;            /* 当前number */
int cc, ll;         /* getch使用的计数器，cc表示当前字符(ch)的位置，ll为line length缓存区长度 */
char line[81];      /* 读取行缓冲区 */
char a[al+1];       /* 临时符号，多出的一个字节用于存放0 */
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
char fname[al];


void error(int n); 
void getsym();
void getch();
void init();
void block(int tx);
void factor(int* ptx);
void term(int* ptx);
void condition(int* ptx);
void expression(int* ptx);
void statement(int* ptx);
void vardeclaration(int* ptx);
void constdeclaration(int* ptx);
int position(char* idt, int tx);
void enter(enum object k, int* ptx);


/* 主程序开始 */
int main()
{
    printf("Input pl/0 file?   ");
	scanf("%s", fname);		/* 输入文件名 */

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

	if (sym == main)
    {
        if (sym == lbrace)
		{
			getsym();
			int i = declaration_list();		/* 处理分程序 */
			statement_list(&i);		/* 处理分程序 */
			if (sym != rbrace)
			{
				error(92);	//格式错误，应是右大括号
			}
			else 
			{
				printf("\n===Parsing success!===\n");
				fprintf(foutput,"\n===Parsing success!===\n");
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
	for (i=0; i<=255; i++)
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

	

	/* 设置保留字名字,按照字母顺序，便于二分查找 */
	strcpy(&(word[0][0]), "begin");
	strcpy(&(word[1][0]), "call");
	strcpy(&(word[2][0]), "const");
	strcpy(&(word[3][0]), "do");
	strcpy(&(word[4][0]), "end");
	strcpy(&(word[5][0]), "if");
	strcpy(&(word[6][0]), "odd");
	strcpy(&(word[7][0]), "procedure");
	strcpy(&(word[8][0]), "read");
	strcpy(&(word[9][0]), "then");
    strcpy(&(word[10][0]), "var");
    strcpy(&(word[11][0]), "while");
    strcpy(&(word[12][0]), "write");

    strcpy(&(word[12][0]), "main");
    strcpy(&(word[12][0]), "else");

	/* 设置保留字符号 */
	wsym[0] = beginsym;	
	wsym[1] = callsym;
	wsym[2] = constsym;
	wsym[3] = dosym;
	wsym[] = elsesym;
    wsym[4] = endsym;
	wsym[5] = ifsym;
	wsym[6] = oddsym;
	wsym[7] = procsym;
	wsym[8] = readsym;
    wsym[9] = thensym;
    wsym[10] = varsym;  
	wsym[11] = whilesym;
	wsym[12] = writesym;   
}


/* 
 *	出错处理，打印出错位置和错误编码
 *  遇到错误就退出语法分析
 */	
void error(int n)
{
	char space[81];
	memset(space,32,81);

	space[cc-1]=0; /* 出错时当前符号已经读完，所以cc-1 */
	
	printf("%s^%d\n", space, n);
	fprintf(foutput,"%s^%d\n", space, n);
	
	exit(1);
}

/*
 * 过滤空格，读取一个字符
 * 每次读一行，存入line缓冲区，line被getsym取空后再读一行
 * 被函数getsym调用
 */
void getch()
{
	if (cc == ll) /* 判断缓冲区中是否有字符，若无字符，则读入下一行字符到缓冲区中 */
	{
		if (feof(fin))
		{
			printf("Program incomplete!\n");
			exit(1);
		}
		ll = 0;
		cc = 0;
	
		ch = ' ';
		while (ch != 10)
		{
            if (EOF == fscanf(fin,"%c", &ch))   
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
}

/* 
 * 词法分析，获取一个符号
 */
void getsym()
{
	int i,j,k;

	while (ch == ' ' || ch == 10 || ch == 9)	/* 过滤空格、换行和制表符 */
	{
		getch();
	}
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) /* 当前的单词是标识符或是保留字 */
	{			
		k = 0;
		do {
			if(k < al)
			{
				a[k] = ch;
				k++;
			}
			getch();
		}while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'));
		a[k] = 0;
		strcpy(id, a);
		i = 0;
		j = norw - 1;
		do {    /* 搜索当前单词是否为保留字，使用二分法查找 */
			k = (i + j) / 2;
			if (strcmp(id,word[k]) <= 0)
			{
			    j = k - 1;
			}
			if (strcmp(id,word[k]) >= 0)
			{
			    i = k + 1;
			}
		} while (i <= j);
		if (i-1 > j) /* 当前的单词是保留字 */
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
						if(ch == '=')	//检测==符号
						{
							getch();
							if (ch == '=')
							{
								sym = equal;
								getch();
							}
						}
						else{
							if(ch == '!')	//检测不等于！=符号
							{
								getch();
								if(ch=='='){
									sym = nequal;
									getch();
								}
							}
							else{
								sym = ssym[ch];		/* 当符号不满足上述条件时，全部按照单字符符号处理 */                   
		                        if (sym != period)  
		                        {
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



int declaration_list(int tx)
{
	while(sym == intsym || sym == charsym)
	{
		declaration_stat(&tx);
	}
	return tx;
}


void declaration_stat(int* ptx)
{
	type();
	if (sym == ident)
	{
		getsym();
		if( sym == semicolon)
		{
			enter(procedure, ptx, 0);	/* 填写符号表 */
			getsym();
		}
		else if (sym == lbracket)
		{
			getsym();			
			if (sym == number)
			{
				enter(procedure, ptx, num);	/* 填写符号表 *///----------------------------------------------
				getsym();
			}
	        else 
				error();	/* 数组中间应为number */


	        if (sym == rbracket)
	        {
	        	getsym();
	        	if(sym == semicolon)
	        	{
	        		getsym();
	        	}
	        	else
	        		error();	/* 格式错误，数组声明完应为分号 */
	        }
	        else
	        	error(93);	/* 格式错误，应是右中括号 */

		}
		else
			error();	/* 格式错误，ID后应为分号或左括号 */
	}
	else
		error();	//格式错误，type后应为ID
}

void type(int *ptx)
{
	if (sym == intsym || sym == charsym)
	{	
		vardeclaration(ptx);
		getsym();
	}
	else
	{
		error();	//type只能是int或char
	}
}

void statement_list(int* ptx)
{
	while( sym == ifsym || sym == whilesym || sym == readsym || sym == writesym ||
			sym == lparen || sym == semicolon || sym == ident || sym == number)
	{
		statement(ptx);
	}
}

void statement(int *ptx)
{
	if( sym == ifsym )	//statement 是 if_stat
	{
		getsym();
		if( sym == lparen)
		{
			expression(ptx);
			if(sym == rparen)
			{
				getsym();
				statement(ptx);
				if( sym == elsesym )
				{
					getsym();
					statement(ptx);
				}
			}
			else
				error();	//expression后应为右括号
		}
		else
			error();	//if后应为左括号
	}
	else if( sym  == whilesym )	//statement 是 while_stat
	{
		getsym();
		if( sym == lparen )
		{
			getsym();
			expression(ptx);
			if(sym == rparen)
			{
				getsym();
				statement(ptx);
			}
			else
				error();	//expression后应为右括号
		}
		else
			error();	//while后应为左括号
	}
	else if( sym == readsym )	//statement 是 read_stat
	{
		getsym();
		var(ptx);
		if( sym == semicolon )
		{
			getsym();
		}
		else
			error();	//少了分号
	}
	else if( sym == writesym )	//statement 是 write_stat
	{
		getsym();
		expression(ptx);
		if( sym == semicolon )
		{
			getsym();
		}
		else
			error();	//少了分号
	}
	else if( sym == lbrace )	//statement 是 compound_stat
	{
		getsym();
		statement_list(ptx);
		if( sym == rbrace)
		{
			getsym();
		}
		else
			error();	//compound_stat的最后应为右大括号

	}
	else if ( sym == semicolon)	//statement 是 expression_stat 之二
	{
		getsym();
	}
	else{	//剩下的就是 statement 是 expression_stat 之一，或是其他错误情况
		expression(ptx);
		if( sym == semicolon )
		{
			getsym();
		}
		else
			error();	//格式错误，不符合expression_stat格式，错误的statement表达式
	}

}


void expression(int *ptx)
{
	if( sym == lparen || sym == number ){
		simple_expr();
	}
	else if(sym == ident){
		getsym();
		if(sym == lbracket){	//左中括号，是数组形式
			getsym();
			expression(ptx);
			if(sym == rbracket)		//右中括号，数组结束
			{
				getsym();
			}
			else
				error();	//数组右边必须是右中括号
		}

		if(sym==eql){	//expression之一
			getsym();
			expression();
		}
		else if(sym == times || sym == slash){	//simple_expr嵌套之factor读完var
			getsym();
			factor();
		}
		else if(sym == plus || sym == minus){
			getsym();
			term();
		}
		else if(sym == gtr || sym == lss || sym == geq || sym == leq || sym == equal || sym == nequal){
			getsym();
			additive_expr(ptx);
		}
		else
			error();
	}
	else{
		error();	//first（expression）只能是ident、lparen、number
	}
}

void simple_expr(int *ptx)
{
	additive_expr(ptx);
	if(sym == gtr || sym == lss || sym == geq || sym == leq || sym == equal || sym == nequal){
		getsym();
		additive_expr(ptx);
	}
}

void additive_expr(int *ptx)
{
	term();
	while( sym == plus || sym == minus){
		getsym();
		term(ptx);
	}
}

void term(int *ptx)
{
	factor(ptx);
	while(sym == times || sym == slash){
		getsym();
		factor(ptx);
	}
}

void factor(int *ptx)
{
	if(sym == lparen){
		getsym();
		expression(ptx);
		if(sym == rparen){
			getsym();
		}
		else
			error();	//expression后应为右括号
	}
	else if(sym == ident){
		var(ptx);
	}
	else if(sym == number){
		getsym();
	}
	else
		error();	//factor元素为三种
}


/* 
 * 编译程序主体
 *
 * tx:     符号表当前尾指针 
 */
void block(int tx)
{
	int i;

	if (sym == constsym)	/* 遇到常量声明符号，开始处理常量声明 */
	{
		getsym();
		constdeclaration(&tx);	
	    while (sym == comma)  /* 遇到逗号继续定义常量 */
		{
			getsym();
			constdeclaration(&tx);
		}
		if (sym == semicolon) /* 遇到分号结束定义常量 */
		{
			getsym();
		}
		else
		{
            error(5);   /* 漏掉了分号 */
        }			
	}

	if (sym == varsym)		/* 遇到变量声明符号，开始处理变量声明 */
	{
		getsym();
		vardeclaration(&tx);
		while (sym == comma) 
		{
			getsym();
			vardeclaration(&tx);
		}
		if (sym == semicolon)
		{
			getsym();
		}
		else
        {
			error(5); /* 漏掉了分号 */
		}			
	}

	while (sym == procsym) /* 遇到过程声明符号，开始处理过程声明 */
	{
		getsym();
		if (sym == ident)
		{
			enter(procedure, &tx);	/* 填写符号表 */
			getsym();
		}
        else 
        {
			error(4);	/* procedure后应为标识符 */
        }
		if (sym == semicolon)
		{
			getsym();
		}
		else 
        {
            error(5);	/* 漏掉了分号 */
        }
		
		block(tx); /* 递归调用 */
            
		if(sym == semicolon)
		{
			getsym();				
		}
		else 
        {
            error(5);	/* 漏掉了分号 */
        }
	}
	statement(&tx);		
}



/* 
 * 在符号表中加入一项 
 *
 * k:      标识符的种类为const，var或procedure
 * ptx:    符号表尾指针的指针，为了可以改变符号表尾指针的值 
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

/*
 * 常量声明处理 
 */
void constdeclaration(int* ptx)
{
	if (sym == ident)
	{
		getsym();
		if (sym == eql)
		{
			getsym();
			if (sym == number)
			{
				enter(constant, ptx);
				getsym();
			}
			else
            {
                error(2);	/* 常量声明中的=后应是数字 */
            }
		}
		else 
        {
            error(3);	/* 常量声明中的标识符后应是= */
        }
	}
	else 
    {
        error(4);	/* const后应是标识符 */
    }
}

/*
 * 变量声明处理 
 */
void vardeclaration(int* ptx)
{
	if (sym == ident)
	{
		enter(variable, ptx);	// 填写符号表
		getsym();
	}
    else 
	{
		error(4);	/* var后面应是标识符 */
	}	
}


/*
 * 语句处理 
 */
void statement(int* ptx)
{
	int i;

	if (sym == ident)	/* 准备按照赋值语句处理 */
	{
		i = position(id, *ptx);/* 查找标识符在符号表中的位置 */
		if (i == 0)
		{
			error(11);	/* 标识符未声明 */
		}
		else
		{
			if(table[i].kind != variable)
            {
				error(12);	/* 赋值语句中，赋值号左部标识符应该是变量 */
				i = 0;
			}
            else
            {
                getsym();
		        if(sym == becomes) 
		        {
			        getsym();
		        }
		        else 
		        {
			        error(13);	/* 没有检测到赋值符号 */
		        }		       
		        expression(ptx);	/* 处理赋值符号右侧表达式 */		       
            }
        }
	}
	else
	{
		if (sym == readsym)	/* 准备按照read语句处理 */
		{
			getsym();
			if (sym != lparen)
			{
				error(34);	/* 格式错误，应是左括号 */
			}
			else
			{
				do {
					getsym();
					if (sym == ident)
					{
						i = position(id, *ptx);	/* 查找要读的变量 */
					}
					else
					{
						i = 0;
					}

					if (i == 0)
					{
						error(35);	/* read语句括号中的标识符应该是声明过的变量 */
					}
					
					getsym();

				} while (sym == comma);	/* 一条read语句可读多个变量 */
			}
			if(sym != rparen) 
			{
				error(33);	/* 格式错误，应是右括号 */				
			}
			else
			{
				getsym();
			}
		}
		else
		{
			if (sym == writesym)	/* 准备按照write语句处理 */
			{
				getsym();
				if (sym == lparen)
				{
					do {
						getsym();						
						expression(ptx);	/* 调用表达式处理 */						
					} while (sym == comma);  /* 一条write可输出多个变量的值 */
					if (sym != rparen)
					{
						error(33);	/* 格式错误，应是右括号 */
					}
					else
					{
						getsym();
					}
				}				
			}
			else
			{
				if (sym == callsym)	/* 准备按照call语句处理 */
				{
					getsym();
					if (sym != ident)
					{
						error(14);	/* call后应为标识符 */
					}
					else
					{
						i = position(id, *ptx);
						if (i == 0)
						{
							error(11);	/* 过程名未找到 */
						}
						else
						{
							if (table[i].kind != procedure)
							{
								error(15);	/* call后标识符类型应为过程 */
							}
						}
						getsym();
					}
				}
				else
				{
					if (sym == ifsym)	/* 准备按照if语句处理 */
					{
						getsym();						
						condition(ptx); /* 调用条件处理 */
						if (sym == thensym) 
						{
							getsym();
						}
						else
						{
							error(16);	/* 缺少then */
						}
						statement(ptx);	/* 处理then后的语句 */						
					}
					else
					{
						if (sym == beginsym)	/* 准备按照复合语句处理 */
						{
							getsym();													
							statement(ptx); /* 对begin与end之间的语句进行分析处理 */
							/* 如果分析完一句后遇到语句开始符或分号，则循环分析下一句语句 */
							while (sym == semicolon) 
							{
								getsym();
								statement(ptx);
							}
							if(sym == endsym) 
							{
								getsym();
							}
							else 
							{
								error(17);	/* 缺少end */
							}
						}
						else
						{
							if (sym == whilesym)	/* 准备按照while语句处理 */
							{
								getsym();								
								condition(ptx);	/* 调用条件处理 */								
								if (sym == dosym)
								{
									getsym();
								}
								else 
								{
									error(18);	/* 缺少do */
								}
								statement(ptx);	/* 循环体 */								
							}                            
						}
					}
				}
			}
		}
	}
}

/*
 * 表达式处理 
 */
void expression(int* ptx)
{
	if(sym == plus || sym == minus)	/* 表达式开头有正负号，此时当前表达式被看作一个正的或负的项 */
	{	
		getsym();		
		term(ptx);	/* 处理项 */		
	}
	else	/* 此时表达式被看作项的加减 */
	{
		term(ptx);	/* 处理项 */
	}
	while (sym == plus || sym == minus)
	{
		getsym();	
		term(ptx);	/* 处理项 */		
	}	
}

/*
 * 项处理 
 */
void term(int* ptx)
{
	factor(ptx);	/* 处理因子 */
	while(sym == times || sym == slash)
	{
		getsym();
		factor(ptx);		
	}	
}

/* 
 * 因子处理 
 */
void factor(int* ptx)
{
	int i;
	
	if(sym == ident)	/* 因子为常量或变量 */
	{
		i = position(id, *ptx);	/* 查找标识符在符号表中的位置 */
		if (i == 0)
		{
			error(11);	/* 标识符未声明 */
		}
		else
		{
			if (table[i].kind == procedure)
			{
				error(21);	/* 不能为过程 */						
			}
		}
		getsym();
	}	
	else
	{
		if(sym == number)	/* 因子为数 */
		{
			getsym();
		}
		else
		{
			if (sym == lparen)	/* 因子为表达式 */
			{
				getsym();
				expression(ptx);
				if (sym == rparen)
				{
					getsym();
				}
				else 
				{
					error(22);	/* 缺少右括号 */
				}
			}	
			else
			{
				error(23);
			}
		}
	}		
}

/* 
 * 条件处理 
 */
void condition(int* ptx)
{
	if(sym == oddsym)	/* 准备按照odd运算处理 */
	{
		getsym();
		expression(ptx);
	}
	else
	{
		expression(ptx); 
		if (sym != eql && sym != neq && sym != lss && sym != leq && sym != gtr && sym != geq)
		{
			error(20); /* 应该为关系运算符 */
		}
		else
		{			
			getsym();
			expression(ptx);			
		}
	}
}
