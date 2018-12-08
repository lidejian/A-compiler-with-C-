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
	charsym, selfplus,selfminus,repeatsym,untilsym,
	mod
};

#define symnum 46

/* ���ű��е����� */
enum object {
	integer,
	character,
};

/* ���������ָ�� */
enum fct {
	lit, opr, lod,
	sto, cal, ini,
	jmp, jpc, sta,
	loa, cpy, chg,
};
#define fctnum 12

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
char tem_id[al + 1];	/*�����ʱid�ĸ������Ա��������ʹ��*/
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
	int idx;			/* ��������飬��������±� */
	int val;            /* ��ֵ */
	int adr;            /* ��ַ */
};

struct tablestruct table[txmax]; /* ���ű� */

FILE* fin;      /* ����Դ�ļ� */
FILE* ftable;	/* ������ű� */
FILE* fcode;    /* ������������ */
FILE* foutput;  /* ����ļ�������ʾ�⣨���д������ж�Ӧ�����ɴ����׵�ַ�����޴� */
FILE* fresult;  /* ���ִ�н�� */
char fname[al] = "test.txt";			//------------------------------------����ʹ�ã�����޸�-----------------------------------

int is_char_flag = 0;


void error(int n);
void getsym();
int getch();
void init();
void gen(enum fct x, int y, int z);
int declaration_list(int tx,int dx);
void declaration_stat(int* ptx,int *pdx);
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
			gen(opr, 0, 0);	                    /* ÿ�����̳��ڶ�Ҫʹ�õ��ͷ����ݶ�ָ�� */
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
	strcpy(&(mnemonic[sta][0]), "sta");
	strcpy(&(mnemonic[loa][0]), "loa");
	strcpy(&(mnemonic[cpy][0]), "cpy");
	strcpy(&(mnemonic[chg][0]), "chg");
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
*	-1 �ļ���β
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
				printf("    %d int   %s ", i, table[i].name);
				printf("addr=%d\n", table[i].adr);
				fprintf(ftable, "    %d var   %s ", i, table[i].name);
				fprintf(ftable, "addr=%d\n", table[i].adr);
				break;
			case character:
				printf("    %d char   %s ", i, table[i].name);
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
					enter(integer, ptx, 1, pdx);		/* ��д���ű� */
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
	while (sym == ifsym || sym == whilesym || sym == repeatsym || sym == readsym || sym == writesym || sym == lbrace ||
		sym == lparen || sym == semicolon || sym == ident || sym == number || sym == selfminus || sym == selfplus)
	{
		statement(ptx);
	}
}

void statement(int *ptx)
{
	int tem;
	int cx0;	//��¼��ǰ������ţ�����ʱʹ��
	int cx1;	//����2
	if (sym == ifsym)	//statement �� if_stat
	{
		getsym();
		if (sym == lparen)
		{
			getsym();
			expression(ptx);

			cx0 = cx;
			gen(jpc, 0, 0);	//����������ʱ��ת�������0�����淭����statement����л���

			if (sym == rparen)
			{
				getsym();
				statement(ptx);
				cx1 = cx;
				gen(jmp, 0, 0);	//ifִ����֮����ת��else֮�⣻�����0�����淭����statement����л���
				code[cx0].a = cx;	//����if�����������Ļ���
				if (sym == elsesym)
				{
					getsym();
					statement(ptx);
					
				}
				code[cx1].a = cx;	//����ifִ����Ļ���
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
			cx0 = cx;
			getsym();
			expression(ptx);
			cx1 = cx;

			gen(jpc, 0, 0);	//����������ʱ��ת�������0�����淭����statement����л���

			if (sym == rparen)
			{
				getsym();
				statement(ptx);
			}
			else
				error(114);	//expression��ӦΪ������

			gen(jmp, 0, cx0);
			code[cx1].a = cx;	//����if�����������Ļ���
		}
		else
			error(115);	//while��ӦΪ������
	}
	else if (sym == repeatsym)	//statement �� repeat_stat---------------------------------------------
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

				gen(jpc, 0, cx0);	//������ת��������������ʱ,��������ѭ��

				if (sym == rparen)
				{
					getsym();
					if (sym == semicolon) {
						getsym();
					}
					else
						error(136);	//�����ź�Ӧ�ø��ֺ�
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
			i = position(id, *ptx, 0);
			if (i == 0)
			{
				error(119);	//��ʶ��δ����
			}
			getsym();
			int bracket_flag = 0;
			int bracket_i = 0;
			if (sym == lbracket) {	//�������ţ���������ʽ
				bracket_flag = 1;
				strcpy(tem_id, id);
				getsym();
				tem = expression(ptx);
				bracket_i = position(tem_id, *ptx, tem);
				if (bracket_i == 0)
					error(126);	/* ��ʶ��δ���� */
				if (sym == rbracket)		//�������ţ��������
				{
					getsym();
				}
				else
					error(120);	//�����ұ߱�������������
			}

			gen(opr, 0, 16);	/* ��������ָ���ȡֵ��ջ�� */
			if (bracket_flag != 0)
				gen(sta, 0, table[i].adr);
			else
				gen(sto, 0, table[i].adr);/* ��ջ���������������Ԫ�� */

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
		if(is_char_flag)
			gen(opr, 0, 18);	/* ��������ַ�ָ����ջ����ֵ */
		else
			gen(opr, 0, 14);	/* �����������ָ����ջ����ֵ */
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


int expression(int *ptx)
{
	static int equal_flag = 0;	//���þ�̬����ʵ��������ֵ
	int i,tem;
	int express_flag=0;
	int tll = ll;
	int tcc = cc;
	int tsym = sym;
	int tch = ch;
	char tid[al + 1];
	strcpy(tid, id);
	int ans;

	if (sym == ident) {
		i = position(id, *ptx, 0);
		if (i == 0)
			error(126);	/* ��ʶ��δ���� */
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
			//	error(0);	//��������û������������֮���
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
			error(126);	/* ��ʶ��δ���� */

		getsym();	//�Ѿ�ȷ��ǰ����ident�����Զ�����һ��
		if (sym == lbracket) {	//���ident��һ�����������ţ������������ʽ
			bracket_flag = 1;
			strcpy(tem_id, id);
			getsym();
			tem = expression(ptx);
			bracket_i = position(tem_id, *ptx, tem);
			if (bracket_i == 0)
				error(126);	/* ��ʶ��δ���� */
			if (equal_flag)		//���������ߴ������ȣ�����lod��ʱ����Ҫ���ݵ�ǰ��ջ���ҵ�Ҫloa��ֵ����˸���һ�顣
				gen(cpy, 0, 0);
			if (sym == rbracket) {
				getsym();
			}
			else
				error(0);	//Ӧ����������������ƥ��
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
		if (equal_flag != 0) {			//����������ȣ��������
			if (bracket_flag != 0) {
				gen(loa, 0, table[i].adr);
			}
			else
				gen(lod, 0, table[i].adr);
		}
	}
	else	//����var=expression��ʽ
		ans = simple_expr(ptx);
	return ans;
}

int simple_expr(int *ptx)
{
	int i;
	int ans = additive_expr(ptx);

	bool flag[symnum];	//���з��ŵ���ʱ���
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
			gen(opr, 0, 12);	/* ���ɴ���ָ�� */
		if (flag[lss])
			gen(opr, 0, 10);	/* ����С��ָ�� */
		if (flag[geq])
			gen(opr, 0, 11);	/* ���ɴ��ڵ���ָ�� */
		if (flag[leq])
			gen(opr, 0, 13);	/* ����С�ڵ���ָ�� */
		if (flag[equal])
			gen(opr, 0, 8);		/* �����ж����ָ�� */
		if (flag[nequal])
			gen(opr, 0, 9);		/* �����жϲ���ָ�� */

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
	int ans = term(ptx);	//���յ�һ��������
	int t;	//����еڶ��������������յڶ���������
	
	bool flag[symnum];	//���з��ŵ���ʱ���
	for (i = 0; i<symnum; i++)
	{
		flag[i] = false;
	}

	while (sym == plus || sym == minus ) {
		flag[plus] = (sym == plus) ? true : false;	//���м�¼+������-�ţ��ڶ���term���������ָ�����
		flag[minus] = (sym == minus) ? true : false;

		getsym();
		t = term(ptx);

		if (flag[plus]) {
			gen(opr, 0, 2);		/* ���ɼӷ�ָ�� */
			ans += t;
		}
		if (flag[minus]) {
			gen(opr, 0, 3);		/* ���ɼ���ָ�� */
			ans -= t;
		}

		flag[plus] = 0;		//����ǽ��л�ԭ
		flag[minus] = 0;
	}
	return ans;
}

int term(int *ptx)
{
	int i;
	int ans = factor(ptx);	//���յ�һ��������
	int t;	//����еڶ��������������յڶ���������

	bool flag[symnum];	//���з��ŵ���ʱ���
	for (i = 0; i<symnum; i++)
	{
		flag[i] = false;
	}
	while (sym == times || sym == slash || sym == mod) {
		flag[times] = (sym == times) ? true : false;
		flag[slash] = (sym == slash) ? true : false;
		flag[mod] = (sym == mod) ? true : false;

		getsym();
		t = factor(ptx);	//���յڶ���������
		
		if (flag[times]) {
			gen(opr, 0, 4);		/* ���ɳ˷�ָ�� */
			ans *= t;
		}
		if (flag[slash]) {
			gen(opr, 0, 5);		/* ���ɳ���ָ�� */
			ans /= t;
		}
		if (flag[mod]) {
			gen(opr, 0, 17);	/* ����ģ��ָ�� */
			ans %= t;
		}

		flag[times] = 0;		//��ԭ
		flag[slash] = 0;
		flag[mod] = 0;
	}
	return ans;
}

int factor(int *ptx)
{
	int i;
	int tem;

	bool flag[symnum];	//���з��ŵ���ʱ���
	for (i = 0; i<symnum; i++)
	{
		flag[i] = false;
	}

	if (sym == lparen) {
		getsym();
		tem = expression(ptx);
		if (sym == rparen) {
			getsym();
			return tem;
		}
		else
			error(129);	//expression��ӦΪ������
	}
	else if (sym == ident) {
		i = position(id, *ptx, 0);/* ���ұ�ʶ���ڷ��ű��е�λ�� */
		if (i == 0)
		{
			error(130);	/* ��ʶ��δ���� */
		}
		getsym();
		int bracket_flag = 0;
		int bracket_i = 0;
		if (sym == lbracket) {	//�������ţ���������ʽ
			bracket_flag = 1;
			strcpy(tem_id, id); 
			getsym();
			tem = expression(ptx);
			bracket_i = position(tem_id, *ptx, tem);/* ���������ʶ���ڷ��ű��е�λ�� */
			if (bracket_i == 0)
			{
				error(130);	/* ��ʶ��δ���� */
			}
			if (sym == rbracket)		//�������ţ��������
			{
				getsym();
			}
			else
				error(131);	//�����ұ߱�������������
		}

		if (bracket_flag != 0)
			gen(loa, 0, table[i].adr);	//������ʽ��lod--->loa
		else
			gen(lod, 0, table[i].adr);	//��ͨ������lod

		if (sym == selfminus || sym == selfplus) {		//a++ ��ʽ		ע�ͽ�a++
			if (sym == selfplus) {		//a++,���ջ��Ϊԭ����a			��ʱջ��		...a
				if (bracket_flag != 0) {
					gen(lit, 0, tem);
					gen(cpy, 0, 0);
					gen(loa, 0, table[i].adr);
				}
				else
					gen(lod, 0, table[i].adr);	//�ٴν�a����ջ��			ִ����ջ����	...a a
				gen(lit, 0, 1);				//��1����ջ��				ִ����ջ����	...a a 1
				gen(opr, 0, 2);				//��ջ��ǰ���������			ִ����ջ����	...a a+1
				if (bracket_flag != 0) {
					gen(sta, 0, table[i].adr);
				}
				else
					gen(sto, 0, table[i].adr);//��ջ��a+1����a			ִ����ջ����	...a (��a+1����a,��ʱջ������ԭ����a)
			}
			else {						//a--,ͬa++,						ע�ͽ�a[2]--
				if (bracket_flag != 0) {	//a[2]--					��ʱջ��		...a[2]
					gen(lit, 0, tem);	//								ִ����ջ��	...a[2] 2
					gen(cpy, 0, 0);	//��ջ������һ��						ִ����ջ����	...a[2] 2 2
					gen(loa, 0, table[i].adr);//						ִ����ջ��	...a[2] 2 a[2]
				}
				else
					gen(lod, 0, table[i].adr);
				gen(lit, 0, 1);				//��1����ջ��				ִ����ջ����	...a[2] 2 a[2] 1
				gen(opr, 0, 3);				//��ջ��ǰ���������			ִ����ջ���� ...a[2] 2 a[2]-1
				if (bracket_flag != 0) {
					gen(sta, 0, table[i].adr);	//						ִ����ջ����	...a[2]  (��a[2]-1��Ž�a[2]�ĵ�ַ�ˣ�ջ��Ԫ��Ϊa[2])
				}
				else
					gen(sto, 0, table[i].adr);//��ջ��a+1����a,��ʱջ����ԭ����a
			}
			getsym();
		}

		return table[i].val;
	}
	else if (sym == number) {
		gen(lit, 0, num);	/* ����������ָ�� */
		getsym();
		return num;
	}
	else if (sym == selfminus || sym == selfplus) {	//++a ��ʽ
		flag[selfminus] = (sym == selfminus) ? true : false;
		flag[selfplus] = (sym == selfplus) ? true : false;
		getsym();
		if (sym == ident) {
			i = position(id, *ptx, 0);/* ���ұ�ʶ���ڷ��ű��е�λ�� */
			if (i == 0)
			{
				error(132);	/* ��ʶ��δ���� */
			}
			getsym();

			int bracket_flag = 0;
			int bracket_i = 0;

			if (sym == lbracket) {	//�������ţ���������ʽ
				bracket_flag = 1;
				strcpy(tem_id, id);
				getsym();
				tem = expression(ptx);
				bracket_i = position(tem_id, *ptx, tem);/* ���������ʶ���ڷ��ű��е�λ�� */
				if (bracket_i == 0)
					error(132);	/* ��ʶ��δ���� */

				if (sym == rbracket)		//�������ţ��������
				{
					getsym();
				}
				else
					error(133);	//�����ұ߱�������������
			}

			if (flag[selfplus]) {		//++a,����ջ��Ϊa+1						��ʱջ��		...
				if (bracket_flag != 0) {
					gen(cpy, 0, 0);
					gen(cpy, 0, 0);
					gen(loa, 0, table[i].adr);
				}
				else {
					gen(lod, 0, table[i].adr);//								ִ����ջ����	...a
				}
				gen(lit, 0, 1);				//ԭ��ջ��Ϊa,��1����ջ��				ִ����ջ����	...a 1
				gen(opr, 0, 2);				//��a��1��ӣ��õ�ջ��a+1				ִ����ջ����	...a+1
				if (bracket_flag != 0) {
					gen(sta, 0, table[i].adr);
					gen(loa, 0, table[i].adr);
				}
				else {
					gen(sto, 0, table[i].adr);	//��ջ��a+1����a					ִ����ջ����	...		(��a+1����a)
					gen(lod, 0, table[i].adr);	//�ٽ�a����ջ������ʱ��a��a+1		ִ����ջ����	...a+1	(��ʱ��lod a,�õ�����a+1��ֵ)
				}
				flag[selfplus] = false;	//������


			}
			if (flag[selfminus]) {		//--a��ͬ��			ע�ͽ���--a[3];		��ʱջ��		...3
				if (bracket_flag != 0) {
					gen(cpy, 0, 0);			//����ջ��							ִ����ջ����	...3 3
					gen(cpy, 0, 0);			//����ջ��							ִ����ջ����	...3 3 3
					gen(loa, 0, table[i].adr);//��3����a[3]��ֵ					ִ����ջ����	...3 3 a[3]
				}
				else {
					gen(lod, 0, table[i].adr);
				}
				gen(lit, 0, 1);				//��1����ջ��						ִ����ջ����	...3 3 a[3] 1
				gen(opr, 0, 3);				//��a��1������õ�ջ��a-1				ִ����ջ����	...3 3 a[3]-1
				if (bracket_flag != 0) {
					gen(sta, 0, table[i].adr);	//��ջ������a[3]					ִ����ջ����	...3   (��ʱa[3]��ֵ�Ѿ���a[3]-1)
					gen(loa, 0, table[i].adr);	//��3����a[3]��ֵ				ִ����ջ����	...a[3]	(��ʱ��ջ��a[3]�Ѿ���a[3]-1��)
				}
				else {
					gen(sto, 0, table[i].adr);
					gen(lod, 0, table[i].adr);
				}
				flag[selfminus] = false;
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
void enter(enum object k, int* ptx, int idx, int *pdx)
{
	int i;
	for (i = 0; i < idx; i++) {
		(*ptx)++;
		strcpy(table[(*ptx)].name, id); /* ���ű��name���¼��ʶ�������� */
		table[(*ptx)].kind = k;
		table[(*ptx)].idx = i;
		table[(*ptx)].val = -1;	//����ʼֵȫ����ʼ��Ϊ-1����ֹ��ʼ��Ϊ0ʱ����read�ı�������������������
		switch (k)
		{
		case integer:	/* ���� */
			table[(*ptx)].adr = (*pdx);
			(*pdx)++;
			break;
		case character:	/* ���� */
			table[(*ptx)].adr = (*pdx);
			(*pdx)++;
			break;
		}
	}
}

/*
* ���ұ�ʶ���ڷ��ű��е�λ�ã���tx��ʼ������ұ�ʶ��
* �ҵ��򷵻��ڷ��ű��е�λ�ã����򷵻�0
*
* id:    Ҫ���ҵ�����
* tx:     ��ǰ���ű�βָ��
*/
int position(char* id, int tx, int idx)
{
	int i;
	strcpy(table[0].name, id);
	table[0].idx = idx;
	i = tx;
	while (strcmp(table[i].name, id) != 0 || table[i].idx!=idx)
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
			case 18:/* ջ���ַ�ֵ��� */
				printf("%c", s[t]);
				fprintf(fresult, "%c", s[t]);
				t = t - 1;
				break;
			}
			break;
		case lod:	/* ȡ��Ե�ǰ���̵����ݻ���ַΪa���ڴ��ֵ��ջ�� */
			t = t + 1;
			s[t] = s[ 1 + i.a];
			break;
		case loa:	/* ��ջ����Ϊ��ַΪa,ƫ��Ϊջ����ֵ   ִ��ǰջ����...3   ִ�к�ջ����...a[3]  ����3�õ�a[3]��ֵ  */
			s[t] = s[1 + s[t] + i.a];
			break;
		case sto:	/* ջ����ֵ�浽��Ե�ǰ���̵����ݻ���ַΪa���ڴ� */
			s[1+i.a] = s[t];
			t = t - 1;
			break;
		case sta:	/* ջ����ֵ�����ַΪ��ջ�����ڴ�		ִ��ǰջ����...3 999   ִ�к�ջ����...  ��999����a[3]   */	
			s[1 + s[t-1] + i.a] = s[t];
			t = t - 2;
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
		case cpy:	/* ��ջ������һ�£�����ջ�� */
			s[t + 1] = s[t];
			t = t + 1;
			break;
		case chg:	/* ����ջ���ʹδ�ջ������ */
			s[t + 1] = s[t - 1];	//����s[t+1]��Ϊ��ʱ����������ջ��s[t-1]�ʹδ�ջ��s[t-2]
			s[t - 1] = s[t - 2];
			s[t - 2] = s[t + 1];
			break;
		}
	} while (p != 0);
	printf("End pl0\n");
	fprintf(fresult, "End pl0\n");
}