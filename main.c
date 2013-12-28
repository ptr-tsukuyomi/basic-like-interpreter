#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>

double ex_atof(const char* s)
{
	int minus = 0;
	double result = 0.0;
	char* point = NULL;
	const char* c = NULL;
	int p = 0;

	if(minus = (s[0] == '-')) ++s;

	point = strstr(s,".");
	p = point == NULL ? 0 : point - (s + strlen(s) - 1);


	for(c = (s + strlen(s) - 1);c != (s - 1);--c)
	{
		if(*c == '.') continue;
		result += (*c - '0') * pow(10,p);
		++p;
	}

	return minus ? -result : result;
}

void ex_ftoa10(double src,char* dest,unsigned int n)
{
	if (src < 0) dest++[0] = '-';
	int div = -log10(src);
	size_t i;

	for(i = 0;i < n;++i,++div)
	{
		if(div == 1) dest[i++] = '.';
		dest[i] = (int)(src * pow(10.0,div)) % 10 + '0';
	}
	dest[i] = '\0';
}

// interface definition

void output(char* s)
{
	printf(s);
}

void error(char* s)
{
	output("Error: ");
	output(s);
}

// tokenize (by line)

typedef struct
{
	char* begin;
	char* pos;
	char* end;
} strinfo;

strinfo init_tokenize(char* str)
{
	strinfo s;
	int i;

	s.begin = s.pos = str;

	for(i = 0;str[i] != '\0';++i)
	{
		switch(str[i])
		{
		case '\r':
			str[i] = ' ';
			break;
		case '\n':
			str[i] = '\0';
			break;
		}
	}

	s.end = str + i;

	return s;
}

char* currentline(const strinfo* info)
{
	return info->pos;
}

int nextline(strinfo* info)
{
	if(info->pos > info->end) return -1;
	if(info->pos < info->begin)
	{
		info->pos = info->begin;
		return 0;
	}
	while(*info->pos++ != '\0');
	return info->pos < info->end ? 0 : -1;
}

int prevline(strinfo* info)
{
	info->pos = info->pos - 2;
	if(info->pos < info->begin) return -1;

	while(info->pos >= info->begin)
	{
		if(*--info->pos == '\0')
		{
			++info->pos;
			return 0;
		}
	}

	info->pos = info->begin;
	return 0;
}

// tokenize (by element)

int isalphaordigit(int c)
{
	return (isalpha(c) != 0) | (isdigit(c) != 0);
}

void currenttoken(const strinfo* info,char* dest,int len)
{
	int alpha = isalphaordigit(info->pos[0]);
	int i;
	if(alpha)
	{ // アルファベットと数値
		for(i = 0;i < (len - 1) && alpha == isalphaordigit(info->pos[i]) && (info->pos + i) < info->end;++i)
		{
			dest[i] = info->pos[i];
		}
	} else { // 記号
		i = 1;
		dest[0] = info->pos[0];
	}
	dest[i] = '\0';
}

int nexttoken(strinfo* info)
{
	int alpha = isalphaordigit(info->pos[0]);
	while(alpha == isalphaordigit(*info->pos))
	{
		++info->pos;
		if(info->pos >= info->end)
		{
			info->pos = info->end - 1;
			return -1;
		}
		if(!alpha)
		{
			return 0;
		}
	}
	return 0;
}

int prevtoken(strinfo* info)
{
	int alpha;
	if(info->pos <= info->begin)
	{
		info->pos = info->begin;
		return -1;
	}
	--info->pos;
	alpha = isalphaordigit(info->pos[0]);
	if(!alpha)
	{
		return 0;
	}

	while(alpha == isalphaordigit(*info->pos))
	{
		--info->pos;
		if(info->pos < info->begin)
		{
			info->pos = info->begin;
			return 0;
		}
	}
	++info->pos;
	return 0;
}

// common

int skipspace(strinfo* si)
{
	char buffer[2];
	do
	{
		if (nexttoken(si) == -1) return -1;
		currenttoken(si, buffer, sizeof(buffer));
	} while (buffer[0] == ' ');
	return 0;
}

int movetosymbol_ex(strinfo* si,char symbol)
{
	int bracket = 0;
	char buffer[2];
	do
	{
		if (nexttoken(si) == -1) return -1;
		currenttoken(si, buffer, sizeof(buffer));
		if (buffer[0] == '(') ++bracket;
		if (buffer[0] == ')') --bracket;
	} while (buffer[0] != symbol || bracket != 0);
	return 0;
}

int movetosymbol(strinfo* si,char symbol)
{
	char buffer[2];
	do
	{
		if (nexttoken(si) == -1) return -1;
		currenttoken(si, buffer, sizeof(buffer));
	} while (buffer[0] != symbol);
	return 0;
}

int equal(const void* s1, const void* s2, size_t size)
{
	size_t i;
	for (i = 0; i < size; ++i)
	{
		if (((const unsigned char*)s1)[i] != ((const unsigned char*)s2)[i]) return 0;
	}
	return 1;
}

void copystring(char* dest,const strinfo* begin,const strinfo* end,size_t length)
{
	size_t len = 0;
	strinfo si = *begin;
	do
	{
		currenttoken(&si, dest + len, length - len);
		len = strlen(dest);
	} while (len < length && (end == NULL || !equal(&si, end, sizeof(si))) && nexttoken(&si) != -1);
}

int strcmp_ignl(const char* s1,const char* s2)
{
	while(1)
	{
		if (*s1 == 0 && *s2 == 0) return 0;
		if (tolower(*s1) != tolower(*s2)) return -1;
		++s1;
		++s2;
	}
}

int movetotoken_ex(strinfo* si,char* s)
{
	int inbracket = 0;
	char buffer[32];
	do
	{
		if (nexttoken(si) == -1) return -1;
		currenttoken(si, buffer, sizeof(buffer));
		if (buffer[0] == '"') inbracket = !inbracket;
	} while (strcmp_ignl(buffer, s) != 0 || inbracket);
	return 0;
}



// variable

struct variable
{
	char name[16];
	void* ptr;
	int* sizes;
};

struct variable variables[20];
int nval = 0;

int add_variable(const char* name,int* sizes)
{
	int i;
	int sizelength = 0;
	int size = 1;
	char temp[16] = { 0 };
	int slength = strlen(name);

	if (nval >= 20) return -1;

	if (sizes != NULL)
	{
		for (i = 0; sizes[i] != 0; ++i)
		{
			size *= sizes[i];
		}
		sizelength = i;
	}

	for (i = 0; i < slength; ++i)
	{
		temp[i] = tolower(name[i]);
	}
	switch (temp[strlen(temp) - 1])
	{
	case '$':
		break;
	default:
		size *= sizeof(double);
		break;
	}
	strncpy(variables[nval].name, temp, sizeof(variables[0].name));

	if (sizes != NULL)
	{
		variables[nval].sizes = malloc(sizeof(int)* (sizelength + 1));
		memcpy(variables[nval].sizes, sizes, sizeof(int)*(sizelength + 1));
	} else {
		variables[nval].sizes = NULL;
	}

	variables[nval].ptr = malloc(size);
	memset(variables[nval].ptr, 0, size);
	++nval;
	return 0;
}

struct variable* get_variable(const char* name)
{
	int i;
	char temp[16] = { 0 };
	int length = strlen(name);
	for (i = 0; i < length; ++i)
	{
		temp[i] = tolower(name[i]);
	}
	for (i = 0; i < sizeof(variables) / sizeof(variables[i]); ++i)
	{
		if (strcmp(variables[i].name, temp) == 0)
		{
			return &variables[i];
		}
	}
	return NULL;
}

void finalize_variable()
{
	int i;
	for (i = 0; i < sizeof(variables) / sizeof(variables[0]); ++i)
	{
		free(variables[i].ptr);
		free(variables[i].sizes);
	}
}

// value

int getarraypos(int[], int[], int);
int getpositions(strinfo*, int[], int);
double eval(strinfo*);
int exec(char*);
double calc(double[], char[][2], int);
double eval_expression(strinfo*, const strinfo*);

int getarraypos(int sizes[],int positions[],int d)
{
	int pos = 0;
	int i;
	for (i = 0; i < d - 1; ++i)
	{
		int n = 1;
		int k;
		for (k = i + 1; k < d; ++k)
		{
			n *= sizes[k];
		}
		pos += n * positions[i];
	}
	pos += positions[d - 1];
	return pos;
}

int getpositions(strinfo* _si,int positions[],int maxd)
{
	int i = 0;
	char buffer[8];
	currenttoken(_si, buffer, sizeof(buffer));
	if (buffer[0] != '(') return -1;

	while (i < maxd)
	{
		nexttoken(_si);
		currenttoken(_si, buffer, sizeof(buffer));
		if (buffer[0] == ' ') continue;
		if (buffer[0] == ',') continue;
		if (buffer[0] == ')') return i;
		if(isalpha(buffer[0])) // 変数
		{
			strinfo begin = *_si;
			movetosymbol_ex(_si, ',');
			prevtoken(_si);
			positions[i] = (int)eval_expression(&begin, _si);
		} else {	// 即値
			positions[i] = atoi(buffer);
		}

		++i;
	}
	return -1;
}

double eval(strinfo* _si)
{
	char s[32];
	const char* c = s;
	currenttoken(_si, s, sizeof(s));
	if (*c == '-') ++c;
	if (isalpha(*c))
	{
		struct variable* v = get_variable(s);
		int slen;
		if(v->sizes == NULL)
		{
			return *((double*)v->ptr);	// 通常の変数
		} else {
			// 以下配列
			int positions[8] = {0};

			for (slen = 0; v->sizes[slen] != 0; ++slen) {}
			nexttoken(_si);
			if (slen != getpositions(_si, positions, sizeof(positions)/sizeof(positions[0]) - 1))
			{
				error("array-position");
				return 0;
			} else {
				return ((double*)v->ptr)[getarraypos(v->sizes, positions, slen)];
			}
		}
	} else {
		strinfo begin = *_si;
		if(!nexttoken(_si))
		{
			currenttoken(_si, s, sizeof(s));
			if(s[0] == '.')
			{
				nexttoken(_si);
				currenttoken(_si, s, sizeof(s));
				if(isdigit(s[0]))
				{
					copystring(s, &begin, _si, sizeof(s));
				} else {
					error("Syntax error");
				}
			} else {
				prevtoken(_si);
				currenttoken(_si, s, sizeof(s));
			}
		}
		return ex_atof(s);
	}
}

double calc(double term[],char op[][2],int nterm)
{
	int i;
	double result;
	for(i = 0;i < nterm - 1;++i)
	{
		switch(op[i][0])
		{
		case '*':
			term[i + 1] = term[i] * term[i + 1];
			term[i] = 0;
			op[i][0] = '+';
			break;
		case '/':
			term[i + 1] = term[i] / term[i + 1];
			term[i] = 0;
			op[i][0] = '+';
			break;
		case '%':
			term[i + 1] = (int)term[i] % (int)term[i + 1];
			term[i] = 0;
			op[i][0] = '+';
			break;
		}
	}
	result = term[0];
	for(i = 0;i < nterm - 1;++i)
	{
		switch(op[i][0])
		{
		case '+':
			result += term[i + 1];
			break;
		case '-':
			result -= term[i - 1];
			break;
		case '<':
		{
			double right = calc(term + i + 1, op + i + 1, nterm - i - 1);
			if (op[i][1] == '=') return result <= right;
			else return result < right;
		}
		case '>':
		{
			double right = calc(term + i + 1, op + i + 1, nterm - i - 1);
			if (op[i][1] == '=') return result >= right;
			else return result > right;
		}
		case '=':
		{
			double right = calc(term + i + 1, op + i + 1, nterm - i - 1);
			return result == right;
		}
		}
	}
	return result;
}

double eval_expression(strinfo* si,const strinfo* end)
{
	double term[10];
	char op[9][2] = {{0}};
	char buffer[32];
	int nterm = 0;
	int nop = 0;

	do {
		currenttoken(si, buffer, sizeof(buffer));
		if (buffer[0] == ' ') continue;
		if (isalphaordigit(buffer[0]))
		{
			term[nterm++] = eval(si);
			nop = 0;
		} else if (buffer[0] == '(') {
			nexttoken(si);
			term[nterm++] = eval_expression(si,end);
			nop = 0;
		} else if (buffer[0] == ')') {
			break;
		} else {
			op[nterm - 1][nop++] = buffer[0];
		}
	} while ((end == NULL || !equal(si, end, sizeof(*si))) && nexttoken(si) != -1);


	return calc(term, op, nterm);
}

// instruction

/* for execution */

strinfo info;
strinfo begin;
strinfo ret[8];
int retpos = 0;
int forced = 0;

/* end */

void output_si(const strinfo* begin,const strinfo* end)
{
	strinfo si = *begin;
	char buffer[128];
	do
	{
		currenttoken(&si, buffer, sizeof(buffer));
		output(buffer);
	} while ((end != NULL && !equal(&si, end, sizeof(si))) && nexttoken(&si) != -1);
}

int print(strinfo* si)
{
	char buffer[32];
	do
	{
		if(!skipspace(si))
		{
			strinfo begin = *si;
			currenttoken(si, buffer, sizeof(buffer));
			if (isalphaordigit(buffer[0]) || buffer[0] == '(')	// 変数
			{
				if (!movetosymbol(si, ';')) prevtoken(si);
				char obuffer[32];
				ex_ftoa10(eval_expression(&begin,si),obuffer,7);
				output(obuffer);
			} else if (buffer[0] == '"') {	// 文字列
				nexttoken(si);
				nexttoken(&begin);
				movetosymbol(si, '"');
				prevtoken(si);
				output_si(&begin, si);
				nexttoken(si);
			} else if (buffer[0] == ';') {
				output(" ");
				if (skipspace(si))
				{
					return 0;
				}
				*si = begin;
			}
		} else {
			break;
		}
	} while (1);
	output("\r\n");
	return 0;
}

int dim(strinfo* si)
{
	char buffer[32];
	char name[16];
	int sizes[8] = {0};
	while(1)
	{
		if (nexttoken(si) == -1)
		{
			error("Syntax error");
			return -1;
		}
		currenttoken(si, buffer, sizeof(buffer));
		if (isalpha(buffer[0])) break;
	}

	strncpy(name, buffer, sizeof(name));

	nexttoken(si);
	getpositions(si, sizes, sizeof(sizes) / sizeof(sizes[0]));

	add_variable(name, sizes);
	return 0;
}

int gosub(strinfo* si)
{
	char buffer[128];
	char name[16];
	char temp[16] = {0};
	skipspace(si);
	strinfo namebegin = *si;
	nexttoken(si);
	copystring(name, &namebegin, si, sizeof(name)); // "*ABCD" style
	ret[retpos++] = info;
	info = begin;
	while (1)
	{
		namebegin = *si = init_tokenize(currentline(&info));
		currenttoken(si, buffer, sizeof(buffer));
		if(buffer[0] == '*' && nexttoken(si) != -1)
		{
			copystring(temp, &namebegin, si, sizeof(temp));
			if (strcmp_ignl(name, temp) == 0) break;
		}
		if(nextline(&info) != 0)
		{
			error("No label");
			return -1;
		}
	}
	return 0;
}

int invalid(strinfo* si)
{
	char temp[12];
	currenttoken(si, temp, sizeof(temp));
	output("invalid instruction (");
	output(temp);
	output(")\r\n");
	return -1;
}

int i_return(strinfo* si)
{
	if(retpos > 0)
	{
		info = ret[--retpos];
		return 0;
	} else {
		return -1;
	}
}

int i_wend(strinfo* si)
{
	forced = 1;
	return i_return(si);
}

int i_if(strinfo* si)
{
	char buffer[64];
	strinfo begin;
	if (skipspace(si)) return -1;
	begin = *si;
	if (movetotoken_ex(si, "then")) return -1;
	prevtoken(si);
	if((int)eval_expression(&begin,si))
	{
		if (nexttoken(si)) return -1;
		if (skipspace(si)) return -1;
		begin = *si;
		if(!movetotoken_ex(si, "else"))
		{
			prevtoken(si);
		}
		copystring(buffer, &begin, si, sizeof(buffer));
		return exec(buffer);
	} else {
		if (movetotoken_ex(si, "else")) return 0;
		skipspace(si);
		begin = *si;
		copystring(buffer, &begin, NULL, sizeof(buffer));
		return exec(buffer);
	}
}

int i_while(strinfo* si)
{
	char buffer[64];
	skipspace(si);
	currenttoken(si, buffer, sizeof(buffer));
	if(!(int)eval_expression(si,NULL))
	{
		int nest = 1;
		do
		{
			strinfo si = init_tokenize(currentline(&info));
			if (nextline(&info)) return -1;
			currenttoken(&si, buffer, sizeof(buffer));
			if (strcmp_ignl(buffer, "WHILE") == 0) ++nest;
			if (strcmp_ignl(buffer, "WEND") == 0) --nest;
		} while (nest != 0);
	} else {
		ret[retpos++] = info;
	}
	return 0;
}

// instruction table

typedef int(*instruction)(strinfo*);

struct insttable
{
	char name[12];					// 12byte
	instruction inst;	// 4byte?
};	// 16byte?

struct insttable const instructions[] =
{
	{ "print", print },
	{ "dim" , dim },
	{ "gosub" , gosub },
	{ "return" , i_return },
	{ "if" , i_if},
	{ "while" , i_while },
	{ "wend" , i_wend }
};

instruction search_instruction(const char* name)
{
	int i;
	char temp[12] = { 0 };
	int length = strlen(name);
	for (i = 0; i < length; ++i)
	{
		temp[i] = tolower(name[i]);
	}

	for (i = 0; i < sizeof(instructions) / sizeof(instructions[0]); ++i)
	{
		if (strcmp(instructions[i].name, temp) == 0)
		{
			return instructions[i].inst;
		}
	}
	return NULL;
}

// interpreter

int exec(char* s)
{
	char buffer[128];
	strinfo line = init_tokenize(s);
	currenttoken(&line, buffer, sizeof(buffer));
	instruction inst = search_instruction(buffer);
	if (inst != NULL)
	{
		return inst(&line);
	} else {
		struct variable* v = get_variable(buffer);
		double* dest;
		if (v == NULL)
		{
			add_variable(buffer, NULL);
			v = get_variable(buffer);
		}
		skipspace(&line);
		currenttoken(&line, buffer, sizeof(buffer));
		if (buffer[0] == '(')	// 配列
		{
			int positions[8];
			int slen;
			int d = getpositions(&line, positions, sizeof(positions) / sizeof(positions[0]));
			int pos;
			for (slen = 0; v->sizes[slen] != 0; ++slen) {}
			if (slen != d)
			{
				error("Dimension missmatch");
				return -1;
			}
			pos = getarraypos(v->sizes, positions, d);
			dest = (double*)v->ptr + pos;
			skipspace(&line);
			currenttoken(&line, buffer, sizeof(buffer));
		} else {
			dest = (double*)v->ptr;
		}

		if (buffer[0] != '=')
		{
			error("Syntax error");
			return -1;
		} else {	//　代入
			skipspace(&line);
			*dest = eval_expression(&line, NULL);
		}
	}
	return 0;
}


void run(char* list)
{
	char buffer[128];
	info = init_tokenize(list);
	begin = info;

	do
	{
		forced = 0;
		strinfo line = init_tokenize(currentline(&info));
		currenttoken(&line, buffer, sizeof(buffer));
		if (strcmp(buffer, "*") == 0) continue;	// ラベル行
		else if(strcmp_ignl(buffer,"END") == 0) {	// end
			return;
		} else {
			copystring(buffer, &line, NULL, sizeof(buffer));
			if (exec(buffer)) return;
		}
	} while(forced || nextline(&info) == 0);
}

// example main function
int main(void)
{
	char str[] =
		"a = 1\n"
		"WHILE a <= 100\n"
		"PRINT a;\n"
		"IF a % 3 = 0 THEN PRINT \"fizz\";\n"
		"IF a % 5 = 0 THEN PRINT \"buzz\";\n"
		"PRINT\n"
		"a = a + 1\n"
		"WEND\n"
		"END\n"
		;
	run(str);
	finalize_variable();

	getchar();
	return 0;
}