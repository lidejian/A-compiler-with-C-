/*
* PL/0 complier program (syntax analysis only) implemented in C
*
* The program has been tested on Visual Studio 2010
*
* ʹ�÷�����
* ���к�����PL/0Դ�����ļ���	 * ���к�����PL/0Դ�����ļ���
* foutput.txt���Դ�ļ�������ʾ�⣨���д�	 * �ش��Ƿ�������������
* һ�����������ֹͣ�﷨����	 * �ش��Ƿ�������ű�
* fcode.txt������������
* foutput.txt���Դ�ļ�������ʾ�⣨���д��͸��ж�Ӧ�����ɴ����׵�ַ�����޴�
* fresult.txt������н��
* ftable.txt������ű�
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define bool int
#define true 1
#define false 0

#define norw 19       /* �����ָ��� */
#define txmax 100     /* ���ű����� */
#define nmax 14       /* ���ֵ����λ�� */
#define al 10         /* ��ʶ������󳤶� */
#define maxerr 30     /* ������������� */
#define amax 2048     /* ��ַ�Ͻ�*/
#define levmax 3      /* ����������Ƕ����������*/
#define cxmax 200     /* ��������������� */
#define stacksize 500 /* ����ʱ����ջԪ�����Ϊ500�� */


/* ���� */
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

/* ���ű��е����� */
enum object {
	integer,
	character,
};

/* ���������ָ�� */
enum fct {
	lit, opr, lod,
	sto, cal, ini,
	jmp, jpc,
};
#define fctnum 8

/* ���������ṹ */
struct instruction
{
	enum fct f; /* ���������ָ�� */
	int l;      /* ���ò���������Ĳ�β� */
	int a;      /* ����f�Ĳ�ͬ����ͬ */
};

bool listswitch;   /* ��ʾ������������ */
bool tableswitch;  /* ��ʾ���ű���� */
char ch;            /* ��ŵ�ǰ��ȡ���ַ���getch ʹ�� */
enum symbol sym;    /* ��ǰ�ķ��� */
char id[al + 1];      /* ��ǰident�������һ���ֽ����ڴ��0 */
int num;            /* ��ǰnumber */
int cc, ll;         /* getchʹ�õļ�������cc��ʾ��ǰ�ַ�(ch)��λ�ã�llΪline length���������� */
int cx;             /* ���������ָ��, ȡֵ��Χ[0, cxmax-1]*/
char line[81];      /* ��ȡ�л����� */
char a[al + 1];       /* ��ʱ���ţ������һ���ֽ����ڴ��0 */
struct instruction code[cxmax]; /* ����������������� */
char word[norw][al];        /* ������ */
enum symbol wsym[norw];     /* �����ֶ�Ӧ�ķ���ֵ */
enum symbol ssym[256];      /* ���ַ��ķ���ֵ */
char mnemonic[fctnum][5];   /* ���������ָ������ */



							/* ���ű�ṹ */
struct tablestruct
{
	char name[al];	    /* ���� */
	enum object kind;	/* ���ͣ�int��char */
	int size;			/* ��������飬��������С */
	int val;            /* ��ֵ����constʹ�� */
	int adr;            /* ��ַ����const��ʹ�� */
};

struct tablestruct table[txmax]; /* ���ű� */

FILE* fin;      /* ����Դ�ļ� */
FILE* ftable;	/* ������ű� */
FILE* fcode;    /* ������������ */
FILE* foutput;  /* ����ļ�������ʾ�⣨���д������ж�Ӧ�����ɴ����׵�ַ�����޴� */
FILE* fresult;  /* ���ִ�н�� */
char fname[al] = "test.txt";			//------------------------------------����ʹ�ã�����޸�-----------------------------------


void error(int n);
void getsym();
int getch();
void init();
void gen(enum fct x, int y, int z);
int declaration_list(int tx,int dx);
void declaration_stat(int* ptx,int *pdx);
void statement_list(int* ptx);
void statement(int *ptx);
void expression(int *ptx);
void simple_expr(int *ptx);
void additive_expr(int *ptx);
void term(int *ptx);
void factor(int *ptx);
void listcode(int cx0);
void listall();
int position(char* idt, int tx);
void enter(enum object k, int* ptx, int s, int* pdx);
void interpret();


/* ������ʼ */
int main()
{
	printf("Input pl/0 file?   ");
	//scanf("%s", fname);		/* �����ļ��� *///------------------------------------����ʹ�ã�����޸�-----------------------------------

	if ((fin = fopen(fname, "r")) == NULL)
	{
		printf("Can't open the input file!\n");
		exit(1);
	}

	ch = fgetc(fin);
	if (ch == EOF)    /* �ļ�Ϊ�� */
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

	printf("List object codes?(Y/N)");	/* �Ƿ������������� */
	//scanf("%s", fname);	//--------------------------------����---------------------------
	fname[0] = 'y';
	listswitch = (fname[0] == 'y' || fname[0] == 'Y');

	printf("List symbol table?(Y/N)");	/* �Ƿ�������ű� */
	//scanf("%s", fname);		//--------------------------------����---------------------------
	fname[0] = 'y';
	tableswitch = (fname[0] == 'y' || fname[0] == 'Y');

	init();		/* ��ʼ�� */
	cc = ll = cx = 0;
	ch = ' ';

	getsym();

	if (sym == mainsym)
	{
		getsym();
		if (sym == lbrace)
		{
			getsym();
			int i = declaration_list(0,3);		/* ����ֳ��� */
			statement_list(&i);		/* ����ֳ��� */
			if (sym != rbrace)
			{
				error(100);	//��ʽ����Ӧ���Ҵ�����
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

				listall();	 /* ������д��� */
				fclose(fcode);

				interpret();	/* ���ý���ִ�г��� */
				fclose(fresult);
			}
		}
		else
			error(101);	//��ʽ����Ӧ���������
	}
	else
		error(102);	//��ʽ����Ӧ��main


	fclose(foutput);
	fclose(fin);

	//system("pause");

	return 0;
}

/*
* ��ʼ��
*/
void init()
{
	int i;

	/* ���õ��ַ����� */
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



	/* ���ñ���������,������ĸ˳�򣬱��ڶ��ֲ��� */
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

	/* ���ñ����ַ��� */
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

	/* ����ָ������ */
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[ini][0]), "int");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");

}


/*
*	��������ӡ����λ�úʹ������
*  ����������˳��﷨����
*/
void error(int n)
{
	char space[81];
	memset(space, 32, 81);

	space[cc - 1] = 0; /* ����ʱ��ǰ�����Ѿ����꣬����cc-1 */

	printf("%s^%d\n", space, n);
	fprintf(foutput, "%s^%d\n", space, n);

	exit(1);
}

/*
* ���˿ո񣬶�ȡһ���ַ�
* ÿ�ζ�һ�У�����line��������line��getsymȡ�պ��ٶ�һ��
* ������getsym����
* ����ֵ��
*	0 ��������
*	1 �ļ���β
*/
int getch()
{
	if (cc == ll) /* �жϻ��������Ƿ����ַ��������ַ����������һ���ַ����������� */
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
* �ʷ���������ȡһ������
*/
void getsym()
{
	int i, j, k;

	while (ch == ' ' || ch == 10 || ch == 9)	/* ���˿ո񡢻��к��Ʊ�� */
	{
		getch();
	}
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) /* ��ǰ�ĵ����Ǳ�ʶ�����Ǳ����� */
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
		do {    /* ������ǰ�����Ƿ�Ϊ�����֣�ʹ�ö��ַ����� */
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
		if (i - 1 > j) /* ��ǰ�ĵ����Ǳ����� */
		{
			sym = wsym[k];
		}
		else /* ��ǰ�ĵ����Ǳ�ʶ�� */
		{
			sym = ident;
		}
	}
	else
	{
		if (ch >= '0' && ch <= '9') /* ��ǰ�ĵ��������� */
		{
			k = 0;
			num = 0;
			sym = number;
			do {
				num = 10 * num + ch - '0';
				k++;
				getch();
			} while (ch >= '0' && ch <= '9'); /* ��ȡ���ֵ�ֵ */
			k--;
			if (k > nmax) /* ����λ��̫�� */
			{
				error(103);
			}
		}
		else
		{
			if (ch == ':')		/* ��⸳ֵ���� */
			{
				getch();
				if (ch == '=')
				{
					sym = becomes;
					getch();
				}
				else
				{
					sym = nul;	/* ����ʶ��ķ��� */
				}
			}
			else
			{
				if (ch == '<')		/* ���С�ڻ�С�ڵ��ڷ��� */
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
					if (ch == '>')		/* �����ڻ���ڵ��ڷ��� */
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
						if (ch == '=')	//���==����  �� �����ĵ��ں�
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
							if (ch == '!')	//��ⲻ���ڣ�=����
							{
								getch();
								if (ch == '=') {
									sym = nequal;
									getch();
								}
							}
							else {
								if (ch == '+') {	//������������++ �� + 
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
									if (ch == '-') {	//����Լ������-- �� - 
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
										if (ch == '/') {	//���ע��//��/
											getch();
											if (ch == '/') {	//��ע��//,����л�����,���¶�ȡsym
												ll = cc = 0;
												getch();
												getsym();
											}
											else if (ch == '*') {	//����ע��/*��һֱ��ȡ,ֱ��*/ֹͣ
												getch();
												while (1)	/* ���˿ո񡢻��к��Ʊ�� */
												{
													while (ch != '*') {
														getch();
														if (ch == '\0')
															break;
													}
													if (getch() == -1)
														error(104);	//�����ļ�ĩβ
													if (ch == '/') {	//��ȡ������ע�ͽ�����*/
														getch();
														getsym();
														break;
													}
													if (ch == '\0')
														error(105);	//ƥ�䵽�����û��ƥ�䵽*/
												}
											}
											else
												sym = ssym['/'];
										}
										else {
											sym = ssym[ch];		/* �����Ų�������������ʱ��ȫ�����յ��ַ����Ŵ��� */
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
* �������������
*
* x: instruction.f;
* y: instruction.l;
* z: instruction.a;
*/
void gen(enum fct x, int y, int z)
{
	if (cx >= cxmax)
	{
		printf("Program is too long!\n");	/* ���ɵ���������������� */
		exit(1);
	}
	if (z >= amax)
	{
		printf("Displacement address is too big!\n");	/* ��ַƫ��Խ�� */
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	cx++;
}

int declaration_list(int tx, int dx)
{
	gen(jmp, 0, 1);
	while (sym == intsym || sym == charsym)
	{
		declaration_stat(&tx,&dx);
	}
	gen(ini, 0, dx);

	int i;
	if (tableswitch)		/* ������ű� */
	{
		for (i = 1; i <= tx; i++)
		{
			switch (table[i].kind)
			{
			case integer:
				printf("    %d var   %s ", i, table[i].name);
				printf("addr=%d\n", table[i].adr);
				fprintf(ftable, "    %d var   %s ", i, table[i].name);
				fprintf(ftable, "addr=%d\n", table[i].adr);
				break;
			}
		}
		printf("\n");
		fprintf(ftable, "\n");
	}

	return tx;
}


void declaration_stat(int* ptx,int *pdx)
{
	int is_int_flag;	//������¼��int����char,
	if (sym == intsym || sym == charsym)	//�����type
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
					enter(integer, ptx, 0, pdx);		/* ��д���ű� */
				else
					enter(character, ptx, 0, pdx);
				getsym();
			}
			else if (sym == lbracket)
			{
				getsym();
				if (sym == number)
				{
					if (is_int_flag == 1)
						enter(integer, ptx, num, pdx);	/* ��д���ű� *///----------------------------------------------
					else
						enter(character, ptx, num, pdx);
					getsym();
				}
				else
					error(106);	/* �����м�ӦΪnumber */


				if (sym == rbracket)
				{
					getsym();
					if (sym == semicolon)
					{
						getsym();
					}
					else
						error(107);	/* ��ʽ��������������ӦΪ�ֺ� */
				}
				else
					error(108);	/* ��ʽ����Ӧ���������� */

			}
			else
				error(109);	/* ��ʽ����ID��ӦΪ�ֺŻ������� */
		}
		else
			error(110);	//��ʽ����type��ӦΪID
	}
	else
	{
		error(111);	//typeֻ����int��char
	}
}



void statement_list(int* ptx)
{
	while (sym == ifsym || sym == whilesym || sym == repeatsym || sym == readsym || sym == writesym ||
		sym == lparen || sym == semicolon || sym == ident || sym == number)
	{
		statement(ptx);
	}
	gen(opr, 0, 0);
}

void statement(int *ptx)
{
	if (sym == ifsym)	//statement �� if_stat
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
				error(112);	//expression��ӦΪ������
		}
		else
			error(113);	//if��ӦΪ������
	}
	else if (sym == whilesym)	//statement �� while_stat
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
				error(114);	//expression��ӦΪ������
		}
		else
			error(115);	//while��ӦΪ������
	}
	else if (sym == repeatsym)	//statement �� repeat_stat---------------------------------------------
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
					error(116);	//expression���������
				}
			}
			else
			{
				error(117);	//until������������
			}
		}
		else
			error(118);	//repeat������until
	}
	else if (sym == readsym)	//statement �� read_stat
	{
		int i;
		getsym();
		if (sym == ident) {
			i = position(id, *ptx);
			if (i == 0)
			{
				error(119);	//��ʶ��δ����
			}
			getsym();
			if (sym == lbracket) {	//�������ţ���������ʽ
				getsym();
				expression(ptx);
				if (sym == rbracket)		//�������ţ��������
				{
					getsym();
				}
				else
					error(120);	//�����ұ߱�������������
			}

			gen(opr, 0, 16);	/* ��������ָ���ȡֵ��ջ�� */
			gen(sto, 0, table[i].adr);	/* ��ջ���������������Ԫ�� */

		}
		else
			error(121);	//read��ӦΪvar,��first(var)={ident}

		if (sym == semicolon)
		{
			getsym();
		}
		else
			error(122);	//���˷ֺ�
	}
	else if (sym == writesym)	//statement �� write_stat
	{
		getsym();
		expression(ptx);
		gen(opr, 0, 14);	/* �������ָ����ջ����ֵ */
		gen(opr, 0, 15);	/* ���ɻ���ָ�� */
		if (sym == semicolon)
		{
			getsym();
		}
		else
			error(123);	//���˷ֺ�
	}
	else if (sym == lbrace)	//statement �� compound_stat
	{
		getsym();
		statement_list(ptx);
		if (sym == rbrace)
		{
			getsym();
		}
		else
			error(124);	//compound_stat�����ӦΪ�Ҵ�����

	}
	else if (sym == semicolon)	//statement �� expression_stat ֮��
	{
		getsym();
	}
	else {	//ʣ�µľ��� statement �� expression_stat ֮һ�����������������
		expression(ptx);
		if (sym == semicolon)
		{
			getsym();
		}
		else
			error(125);	//��ʽ���󣬲�����expression_stat��ʽ�������statement���ʽ
	}

}


void expression(int *ptx)
{
	int i;
	if (sym == selfminus || sym == selfplus)	//++a��ʽ
		getsym();
	if (sym == lparen || sym == number) {
		simple_expr(ptx);
	}
	else if (sym == ident) {
		int single_ident_flag = 1;
		i = position(id, *ptx);/* ���ұ�ʶ���ڷ��ű��е�λ�� */
		if (i == 0)
		{
			error(126);	/* ��ʶ��δ���� */
		}
		getsym();
		if (sym == lbracket) {	//�������ţ���������ʽ
			getsym();
			expression(ptx);
			if (sym == rbracket)		//�������ţ��������
			{
				getsym();
			}
			else
				error(127);	//�����ұ߱�������������
		}
		if (sym == eql) {	//expression֮һ
			single_ident_flag = 0;
			getsym();
			expression(ptx);
			if(i!=0)
				gen(sto, 0, table[i].adr);
		}
		gen(lod, 0, table[i].adr);
		if(sym == selfplus || sym == selfminus) {		//expression ��չ�� expression: var++ | var--
			single_ident_flag = 0;
			getsym();
		}

		int plus_flag = 0, minus_flag = 0;
		int times_flag = 0, slash_flag = 0, mod_flag = 0;

		if (sym == times || sym == slash || sym == mod || sym == plus || sym == minus ) {
			single_ident_flag = 0;
			do {
				plus_flag = (sym == plus) ? 1 : 0;	//���м�¼+������-�ţ��ڶ���term���������ָ�����
				minus_flag = (sym == minus) ? 1 : 0;
				times_flag = (sym == times) ? 1 : 0;
				slash_flag = (sym == slash) ? 1 : 0;
				mod_flag = (sym == mod) ? 1 : 0;

				getsym();
				term(ptx);

				if (plus_flag)
					gen(opr, 0, 2);	/* ���ɼӷ�ָ�� */
				if (minus_flag)
					gen(opr, 0, 3);	/* ���ɼ���ָ�� */
				if (times_flag)
					gen(opr, 0, 4);	/* ���ɳ˷�ָ�� */
				if (slash_flag)
					gen(opr, 0, 5);	/* ���ɳ���ָ�� */
				if (mod_flag)
					gen(opr, 0, 17);	/* ����ȡģָ�� */

				plus_flag = 0;		//����ǽ��л�ԭ
				minus_flag = 0;
				times_flag = 0;
				slash_flag = 0;
				mod_flag = 0;

				while (sym == times || sym == slash || sym == mod) {
					times_flag = (sym == times) ? 1 : 0;
					slash_flag = (sym == slash) ? 1 : 0;
					mod_flag = (sym == mod) ? 1 : 0;

					getsym();
					factor(ptx);

					if(times_flag)
						gen(opr, 0, 4);	/* ���ɳ˷�ָ�� */
					if(slash_flag)
						gen(opr, 0, 5);	/* ���ɳ���ָ�� */
					if(mod_flag)
						gen(opr, 0, 17);	/* ����ȡģָ�� */

					times_flag = 0;
					slash_flag = 0;
					mod_flag = 0;
				}
			} while (sym == plus || sym == minus);
		}
		//if(single_ident_flag == 1)	//�����ԣ�Ϊ����ident
		//	gen(lod, 0, table[i].adr);
	}
	else {
		error(128);	//first��expression��ֻ����ident��lparen��number
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
			error(129);	//expression��ӦΪ������
	}
	else if (sym == ident) {
		i = position(id, *ptx);/* ���ұ�ʶ���ڷ��ű��е�λ�� */
		if (i == 0)
		{
			error(130);	/* ��ʶ��δ���� */
		}
		getsym();
		if (sym == lbracket) {	//�������ţ���������ʽ
			getsym();
			expression(ptx);
			if (sym == rbracket)		//�������ţ��������
			{
				getsym();
			}
			else
				error(131);	//�����ұ߱�������������
		}
		
		if (sym == selfminus || sym == selfplus) {		//a++ ��ʽ
			getsym();
		}

		gen(lod, 0, table[i].adr);

	}
	else if (sym == number) {
		gen(lit, 0, num);
		getsym();
	}
	else if (sym == selfminus || sym == selfplus) {	//++a ��ʽ
		getsym();
		if (sym == ident) {
			i = position(id, *ptx);/* ���ұ�ʶ���ڷ��ű��е�λ�� */
			if (i == 0)
			{
				error(132);	/* ��ʶ��δ���� */
			}
			getsym();
			if (sym == lbracket) {	//�������ţ���������ʽ
				getsym();
				expression(ptx);
				if (sym == rbracket)		//�������ţ��������
				{
					getsym();
				}
				else
					error(133);	//�����ұ߱�������������
			}
		}
		else
			error(134);	//++��--��ӦΪvar
	}
	else
		error(135);	//factorԪ��Ϊ����
}




/*
* �ڷ��ű��м���һ��
*
* k:      ��ʶ��������Ϊint��char
* ptx:    ���ű�βָ���ָ�룬Ϊ�˿��Ըı���ű�βָ���ֵ
* size:	���ű�Ԫ�ش�С����Ϊ0���������������0���ʾ����
*
*/
void enter(enum object k, int* ptx, int s, int *pdx)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, id); /* ���ű��name���¼��ʶ�������� */
	table[(*ptx)].kind = k;
	table[(*ptx)].size = s;
	switch (k)
	{
		case integer:	/* ���� */
			table[(*ptx)].adr = (*pdx);
			(*pdx)++;
			break;
	}
}

/*
* ���ұ�ʶ���ڷ��ű��е�λ�ã���tx��ʼ������ұ�ʶ��
* �ҵ��򷵻��ڷ��ű��е�λ�ã����򷵻�0
*
* id:    Ҫ���ҵ�����
* tx:     ��ǰ���ű�βָ��
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
* ���Ŀ������嵥
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
* �������Ŀ�����
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
* ���ͳ���
*/
void interpret()
{
	int p = 0; /* ָ��ָ�� */
	int b = 1; /* ָ���ַ */
	int t = 0; /* ջ��ָ�� */
	struct instruction i;	/* ��ŵ�ǰָ�� */
	int s[stacksize];	/* ջ */

	printf("Start pl0\n");
	fprintf(fresult, "Start pl0\n");
	s[0] = 0; /* s[0]���� */
	s[1] = 0; /* �������������ϵ��Ԫ����Ϊ0 */
	s[2] = 0;
	s[3] = 0;
	do {
		i = code[p];	/* ����ǰָ�� */
		p = p + 1;
		switch (i.f)
		{
		case lit:	/* ������a��ֵȡ��ջ�� */
			t = t + 1;
			s[t] = i.a;
			break;
		case opr:	/* ��ѧ���߼����� */
			switch (i.a)
			{
			case 0:  /* �������ý����󷵻� */
				t = b - 1;
				p = s[t + 3];
				b = s[t + 2];
				break;
			case 1: /* ջ��Ԫ��ȡ�� */
				s[t] = -s[t];
				break;
			case 2: /* ��ջ�������ջ���������ջԪ�أ����ֵ��ջ */
				t = t - 1;
				s[t] = s[t] + s[t + 1];
				break;
			case 3:/* ��ջ�����ȥջ���� */
				t = t - 1;
				s[t] = s[t] - s[t + 1];
				break;
			case 4:/* ��ջ�������ջ���� */
				t = t - 1;
				s[t] = s[t] * s[t + 1];
				break;
			case 5:/* ��ջ�������ջ���� */
				t = t - 1;
				s[t] = s[t] / s[t + 1];
				break;
			case 6:/* ջ��Ԫ�ص���ż�ж� */
				s[t] = s[t] % 2;
				break;
			case 8:/* ��ջ������ջ�����Ƿ���� */
				t = t - 1;
				s[t] = (s[t] == s[t + 1]);
				break;
			case 9:/* ��ջ������ջ�����Ƿ񲻵� */
				t = t - 1;
				s[t] = (s[t] != s[t + 1]);
				break;
			case 10:/* ��ջ�����Ƿ�С��ջ���� */
				t = t - 1;
				s[t] = (s[t] < s[t + 1]);
				break;
			case 11:/* ��ջ�����Ƿ���ڵ���ջ���� */
				t = t - 1;
				s[t] = (s[t] >= s[t + 1]);
				break;
			case 12:/* ��ջ�����Ƿ����ջ���� */
				t = t - 1;
				s[t] = (s[t] > s[t + 1]);
				break;
			case 13: /* ��ջ�����Ƿ�С�ڵ���ջ���� */
				t = t - 1;
				s[t] = (s[t] <= s[t + 1]);
				break;
			case 14:/* ջ��ֵ��� */
				printf("%d", s[t]);
				fprintf(fresult, "%d", s[t]);
				t = t - 1;
				break;
			case 15:/* ������з� */
				printf("\n");
				fprintf(fresult, "\n");
				break;
			case 16:/* ����һ����������ջ�� */
				t = t + 1;
				printf("?");
				fprintf(fresult, "?");
				scanf("%d", &(s[t]));
				fprintf(fresult, "%d\n", s[t]);
				break;
			case 17:/* ��ջ����ģ��ջ���� */
				t = t - 1;
				s[t] = s[t] % s[t + 1];
				break;
			}
			break;
		case lod:	/* ȡ��Ե�ǰ���̵����ݻ���ַΪa���ڴ��ֵ��ջ�� */
			t = t + 1;
			s[t] = s[ 1 + i.a];
			break;
		case sto:	/* ջ����ֵ�浽��Ե�ǰ���̵����ݻ���ַΪa���ڴ� */
			s[1+i.a] = s[t];
			t = t - 1;
			break;
		case ini:	/* ������ջ��Ϊ�����õĹ��̿���a����Ԫ�������� */
			t = t + i.a;
			break;
		case jmp:	/* ֱ����ת */
			p = i.a;
			break;
		case jpc:	/* ������ת */
			if (s[t] == 0)
				p = i.a;
			t = t - 1;
			break;
		}
	} while (p != 0);
	printf("End pl0\n");
	fprintf(fresult, "End pl0\n");
}