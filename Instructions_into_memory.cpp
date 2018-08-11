#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<string.h>

struct symbltable
{
	char name;
	int address;
	int size;
};
int symbol_table_len = 0;

struct intermediatelan
{
	int lineno;
	int opcode;
	int param[4];
};

int interlen = 0;
enum instr { MOVX = 1, MOV = 2, ADD = 3, SUB = 4, MUL = 5, JMP = 6, IF = 7, EQ = 8 , LT = 9, GT = 10, LTEQ = 11, GTEQ =  12, PRINT = 13, READ =  14 };

enum regcodes { AX=0,BX=1,CX=2,DX=3,EX=4,FX=5,GX=6,HX=7};

void func_starts(FILE *fp);
int equals(char *s1, char *s2);
void read(FILE *fp);
int findMem(FILE *fr, char c);
int findReg(char);

void main()
{
	FILE *fp,*fr;
	int i = 0;
	struct intermediatelan ir[20];
	fp = fopen("assemblyprog.txt", "r");
	fr = fopen("intermediate.bin", "rb+");
	read(fp);
	fseek(fr, 448, SEEK_SET);
	fread(&ir, sizeof(intermediatelan), interlen, fr);
	printf("\n\nsymbol table size: %d\n\n",symbol_table_len);
	for (i = 0; i < interlen; i++)
	{
		printf("\n\nline-no : %d, opcode : %d", ir[i].lineno, ir[i].opcode);
		printf("param: %d %d %d %d", ir[i].param[0], ir[i].param[1], ir[i].param[2], ir[i].param[3]);
	}
	_getch();
}

int equals(char *s1, char *s2)
{
	int i;
	for (i = 0; s1[i]; i++)
	{
		if (s1[i] != s2[i])
			return 0;
	}
	return 1;
}

void read(FILE *fp)
{
	int i, j, size, mstart = 0, systart = 112, zr = 0, val;
	FILE *fr;
	fr = fopen("intermediate.bin", "rb+");
	struct symbltable st;
	char str[20], ch;
	int start = 1;
	while (start){
		fscanf(fp, "%s", &str);
	if (equals(str, "DATA\0"))
	{
		fgetc(fp);
		fscanf(fp, "%c", &ch);
		st.name = ch;
		if (fscanf(fp, "%c", &ch) && ch != '[')
		{
			st.size = 1;
			st.address = mstart;
			fseek(fr, mstart, SEEK_SET);
			fwrite(&zr, sizeof(int), 1, fr);
			mstart = ftell(fr);
			fseek(fr, systart, SEEK_SET);
			fwrite(&st, sizeof(struct symbltable), 1, fr);
			printf("%c -- %d -- %d", st.name, st.address, st.size);
			systart = ftell(fr);
		}
		else
		{
			fscanf(fp, "%d", &size);
			st.size = size;
			st.address = mstart;
			fseek(fr, mstart, SEEK_SET);
			fwrite(&zr, sizeof(int), 1, fr);
			mstart = ftell(fr);
			fseek(fr, systart, SEEK_SET);
			fwrite(&st, sizeof(struct symbltable), 1, fr);
			printf("%c -- %d -- %d", st.name, st.address, st.size);
			systart = ftell(fr);
		}
		symbol_table_len++;

	}
	else if (equals(str, "CONST\0"))
	{
		fgetc(fp);
		fscanf(fp, "%c", &ch);
		st.name = ch;
		st.size = 0;
		fgetc(fp); fgetc(fp); fgetc(fp);
		fscanf(fp, "%d", &val);
		fseek(fr, mstart, SEEK_SET);
		fwrite(&val, sizeof(int), 1, fr);
		mstart = ftell(fr);
		fseek(fr, systart, SEEK_SET);
		fwrite(&st, sizeof(struct symbltable), 1, fr);
		printf("%c -- %d -- %d", st.name, st.address, st.size);
		systart = ftell(fr);
	}
	else if (equals(str, "START:\0"))
	{
		start = 0;
		fclose(fr);
		func_starts(fp);
	}
}
}

void func_starts(FILE *fp)
{
	FILE *fr;
	fr = fopen("intermediate.bin", "rb+");
	char str[20];
	int start = 1, ln = 0,i=0,l,off,param;
	int interstart = 448;
	struct intermediatelan ir;
	while (start){
		ln++;
		memset(&ir, 0, sizeof(struct intermediatelan));
		fscanf(fp, "%s", &str);
		if (equals(str, "MOV\0"))
		{
			ir.lineno = ln;
			param = 0;
			fgetc(fp);
				fscanf(fp, "%[^, ]", &str);
				if (str[1] == 'X')
				{
					ir.opcode = MOVX;
					ir.param[param] = findReg(str[0]);
				}
				else{
					ir.opcode = MOV;
					ir.param[param] = findMem(fr, str[0]);
				}
				fgetc(fp); fgetc(fp);
				param++;
				fscanf(fp, "%[^\n]", &str);
				if (str[1] == 'X')
				{
					ir.param[param] = findReg(str[0]);
				}
				else{
					ir.param[param] = findMem(fr, str[0]);
				}
			fseek(fr, interstart, SEEK_SET);
			fwrite(&ir, sizeof(struct intermediatelan), 1, fr);
			interstart = ftell(fr);
			interlen++;
		}
		else if (equals(str, "READ\0"))
		{
			ir.lineno = ln;
			ir.opcode = READ;
			fgetc(fp);
			fscanf(fp, "%s", &str);
			if (str[1] == 'X')
			{
				ir.param[0] = findReg(str[0]);
			}
			else{
				ir.param[0] = findMem(fr, str[0]);
			}
			fseek(fr, interstart, SEEK_SET);
			fwrite(&ir, sizeof(struct intermediatelan), 1, fr);
			interstart = ftell(fr);
			interlen++;
		}
		else if (equals(str, "PRINT\0"))
		{
			ir.lineno = ln;
			ir.opcode = PRINT;
			fgetc(fp);
			fscanf(fp, "%s", &str);
			if (str[1] == 'X')
			{
				ir.param[0] = findReg(str[0]);
			}
			else{
				ir.param[0] = findMem(fr, str[0]);
			}
			fseek(fr, interstart, SEEK_SET);
			fwrite(&ir, sizeof(struct intermediatelan), 1, fr);
			interstart = ftell(fr);
			interlen++;
		}
		else if (equals(str, "ADD\0"))
		{
			ir.lineno = ln;
			ir.opcode = ADD;
			param = 0;
			fgetc(fp);
			while (param != 2){
				memset(&str, 0, 20);
				fscanf(fp, "%[^, ]", &str);
				if (str[1] == 'X')
				{
					ir.param[param] = findReg(str[0]);
				}
				else{
					ir.param[param] = findMem(fr, str[0]);
				}
				fgetc(fp); fgetc(fp);
				param++;
			}
			fscanf(fp, "%[^\n]", &str);
			if (str[1] == 'X')
			{
				ir.param[param] = findReg(str[0]);
			}
			else{
				ir.param[param] = findMem(fr, str[0]);
			}
			fseek(fr, interstart, SEEK_SET);
			fwrite(&ir, sizeof(struct intermediatelan), 1, fr);
			interstart = ftell(fr);
			interlen++;
		}
		else if (equals(str, "SUB\0"))
		{
			ir.lineno = ln;
			ir.opcode = SUB;
			param = 0;
			fgetc(fp);
			while (param != 2){
				fscanf(fp, "%[^, ]", &str);
				if (str[1] == 'X')
				{
					ir.param[param] = findReg(str[0]);
				}
				else{
					ir.param[param] = findMem(fr, str[0]);
				}
				fgetc(fp); fgetc(fp);
				param++;
			}
			fscanf(fp, "%[^\n]", &str);
			if (str[1] == 'X')
			{
				ir.param[param] = findReg(str[0]);
			}
			else{
				ir.param[param] = findMem(fr, str[0]);
			}
			fseek(fr, interstart, SEEK_SET);
			fwrite(&ir, sizeof(struct intermediatelan), 1, fr);
			interstart = ftell(fr);
			interlen++;
		}
		else if (equals(str, "MUL\0"))
		{
			ir.lineno = ln;
			ir.opcode = MUL;
			param = 0;
			fgetc(fp);
			while (param !=2){
				fscanf(fp, "%[^, ]", &str);
				if (str[1] == 'X')
				{
					ir.param[param] = findReg(str[0]);
				}
				else{
					ir.param[param] = findMem(fr, str[0]);
				}
				fgetc(fp); fgetc(fp);
				param++;
			}
			fscanf(fp, "%[^\n]", &str);
			if (str[1] == 'X')
			{
				ir.param[param] = findReg(str[0]);
			}
			else{
				ir.param[param] = findMem(fr, str[0]);
			}
			fseek(fr, interstart, SEEK_SET);
			fwrite(&ir, sizeof(struct intermediatelan), 1, fr);
			interstart = ftell(fr);
			interlen++;
		}
		else if (equals(str, "END\0"))
		{
			start = 0;
			fclose(fp);
			fclose(fr);
		}
	}
}

int findMem(FILE *fr,char c)
{
	struct symbltable st[20];
	int i,off=0;
	fseek(fr, 112, SEEK_SET);
	fread(&st, sizeof(struct symbltable), symbol_table_len, fr);
	for (i = 0; i < symbol_table_len; i++)
	{
		if (st[i].name == c)
			off = st[i].address;
	}
	return off;
}

int findReg(char c)
{
	int l;
	enum regcodes r;
	switch (c)
	{
	case 'A': l = 0;
		break;
	case 'B':l = 1;
		break;
	case 'C': l = 2;
		break;
	case 'D':l = 3;
		break;
	case 'E':l = 4;
		break;
	case 'F': l = 5;
		break;
	case 'G': l = 6;
		break;
	case 'H':l = 7;
		break;
	}
	return l;
}


