/*
* PL/0 complier program (syntax analysis only) implemented in C
*
* The program has been tested on Visual Studio 2010
*
* ʹ�÷�����
* ���к�����PL/0Դ�����ļ���
* foutput.txt���Դ�ļ�������ʾ�⣨���д�
* һ�����������ֹͣ�﷨����
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


char ch;            /* ��ŵ�ǰ��ȡ���ַ���getch ʹ�� */
enum symbol sym;    /* ��ǰ�ķ��� */
char id[al + 1];      /* ��ǰident�������һ���ֽ����ڴ��0 */
int num;            /* ��ǰnumber */
int cc, ll;         /* getchʹ�õļ�������cc��ʾ��ǰ�ַ�(ch)��λ�ã�llΪline length���������� */
char line[81];      /* ��ȡ�л����� */
char a[al + 1];       /* ��ʱ���ţ������һ���ֽ����ڴ��0 */
char word[norw][al];        /* ������ */
enum symbol wsym[norw];     /* �����ֶ�Ӧ�ķ���ֵ */
enum symbol ssym[256];      /* ���ַ��ķ���ֵ */


							/* ���ű�ṹ */
struct tablestruct
{
	char name[al];	    /* ���� */
	enum object kind;	/* ���ͣ�int��char */
	int size;			/* ��������飬��������С */
};

struct tablestruct table[txmax]; /* ���ű� */

FILE* fin;      /* ����Դ�ļ� */
FILE* foutput;  /* ����ļ�������ʾ�⣨���д� */
char fname[al] = "test.txt";			//------------------------------------����ʹ�ã�����޸�-----------------------------------


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

	init();		/* ��ʼ�� */
	cc = ll = 0;
	ch = ' ';

	getsym();

	if (sym == mainsym)
	{
		getsym();
		if (sym == lbrace)
		{
			getsym();
			int i = declaration_list(0);		/* ����ֳ��� */
			statement_list(&i);		/* ����ֳ��� */
			if (sym != rbrace)
			{
				error(92);	//��ʽ����Ӧ���Ҵ�����
			}
			else
			{
				printf("\n===Parsing success!===\n");
				fprintf(foutput, "\n===Parsing success!===\n");
			}
		}
		else
			error(91);	//��ʽ����Ӧ���������
	}
	else
		error(90);	//��ʽ����Ӧ��main


	fclose(foutput);
	fclose(fin);

	system("pause");

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
				error(30);
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
														error(0);	//�����ļ�ĩβ
													if (ch == '/') {	//��ȡ������ע�ͽ�����*/
														getch();
														getsym();
														break;
													}
													if (ch == '\0')
														error(0);	//ƥ�䵽�����û��ƥ�䵽*/
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
					enter(integer, ptx, 0);		/* ��д���ű� */
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
						enter(integer, ptx, num);	/* ��д���ű� *///----------------------------------------------
					else
						enter(character, ptx, num);
					getsym();
				}
				else
					error(0);	/* �����м�ӦΪnumber */


				if (sym == rbracket)
				{
					getsym();
					if (sym == semicolon)
					{
						getsym();
					}
					else
						error(0);	/* ��ʽ��������������ӦΪ�ֺ� */
				}
				else
					error(93);	/* ��ʽ����Ӧ���������� */

			}
			else
				error(0);	/* ��ʽ����ID��ӦΪ�ֺŻ������� */
		}
		else
			error(0);	//��ʽ����type��ӦΪID
	}
	else
	{
		error(0);	//typeֻ����int��char
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
				error(0);	//expression��ӦΪ������
		}
		else
			error(0);	//if��ӦΪ������
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
				error(0);	//expression��ӦΪ������
		}
		else
			error(0);	//while��ӦΪ������
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
					error(0);	//expression���������
				}
			}
			else
			{
				error(0);	//until������������
			}
		}
		else
			error(0);	//repeat������until
	}
	else if (sym == readsym)	//statement �� read_stat
	{
		int i;
		getsym();
		if (sym == ident) {
			i = position(id, *ptx);
			if (i == 0)
			{
				error(0);	//��ʶ��δ����
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
					error(0);	//�����ұ߱�������������
			}
		}
		else
			error(0);	//read��ӦΪvar,��first(var)={ident}

		if (sym == semicolon)
		{
			getsym();
		}
		else
			error(0);	//���˷ֺ�
	}
	else if (sym == writesym)	//statement �� write_stat
	{
		getsym();
		expression(ptx);
		if (sym == semicolon)
		{
			getsym();
		}
		else
			error(0);	//���˷ֺ�
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
			error(0);	//compound_stat�����ӦΪ�Ҵ�����

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
			error(0);	//��ʽ���󣬲�����expression_stat��ʽ�������statement���ʽ
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
		i = position(id, *ptx);/* ���ұ�ʶ���ڷ��ű��е�λ�� */
		if (i == 0)
		{
			error(0);	/* ��ʶ��δ���� */
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
				error(0);	//�����ұ߱�������������
		}

		if (sym == eql) {	//expression֮һ
			getsym();
			expression(ptx);
		}
		if(sym == selfplus || sym == selfminus) {		//expression ��չ�� expression: var++ | var--
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
		error(0);	//first��expression��ֻ����ident��lparen��number
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
			error(0);	//expression��ӦΪ������
	}
	else if (sym == ident) {
		i = position(id, *ptx);/* ���ұ�ʶ���ڷ��ű��е�λ�� */
		if (i == 0)
		{
			error(0);	/* ��ʶ��δ���� */
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
				error(0);	//�����ұ߱�������������
		}
		
		if (sym == selfminus || sym == selfplus) {		//a++ ��ʽ
			getsym();
		}
	}
	else if (sym == number) {
		getsym();
	}
	else if (sym == selfminus || sym == selfplus) {	//++a ��ʽ
		getsym();
		if (sym == ident) {
			i = position(id, *ptx);/* ���ұ�ʶ���ڷ��ű��е�λ�� */
			if (i == 0)
			{
				error(0);	/* ��ʶ��δ���� */
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
					error(0);	//�����ұ߱�������������
			}
		}
		else
			error(0);	//++��--��ӦΪvar
	}
	else
		error(0);	//factorԪ��Ϊ����
}




/*
* �ڷ��ű��м���һ��
*
* k:      ��ʶ��������Ϊint��char
* ptx:    ���ű�βָ���ָ�룬Ϊ�˿��Ըı���ű�βָ���ֵ
* size:	���ű�Ԫ�ش�С����Ϊ0���������������0���ʾ����
*
*/
void enter(enum object k, int* ptx, int s)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, id); /* ���ű��name���¼��ʶ�������� */
	table[(*ptx)].kind = k;
	table[(*ptx)].size = s;
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