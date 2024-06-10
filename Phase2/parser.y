%{
	#include <stdlib.h>
  	#include <stdio.h>
  	#include <string.h>
	#include "SymbolTable.h"
	#define yylex alpha_yylex
	extern int alpha_yylex();
	void yyerror(void* a,char* yaccProvidedMessasge);
	extern int yylineno;
	extern char* yytext;
	extern FILE* yyin;
	SymbolTable *hashTable;

	int in_func_block_flag= 0;
	int key=-1, scope=0;
	FuncArgs *funcargs;
	char *funcname;
	int funcnamenum=0;
	FuncNames *funcnames;


	void AddArguement(char* name, int scope);
	void ResetArgs();
	void Init();

%}
%lex-param { void* val }
%parse-param { void* val }
%union {
	char* strVal;
	int intVal;
	double realVal;
}

%start program

%token <strVal> ID
%token <integerVal> INT


%token IF 
%token ELSE 
%token WHILE 
%token FOR 
%token FUNCTION 
%token RETURN 
%token BREAK 
%token CONTINUE 
%token AND 
%token NOT 
%token OR 
%token LOCALVAR 
%token TRUE 
%token FALSE 
%token NIL 

%token INSERT
%token PLUS 
%token MINUS 
%token MULT 
%token DIV 
%token MOD 
%token EQUAL 
%token NOT_EQUAL 
%token PLUS_ONE 
%token MINUS_ONE 
%token GREATER 
%token LESS 
%token GREATER_EQUAL 
%token LESS_EQUAL 

%token <realVal> REAL 
%token STRING 

%token L_BRACKET 
%token R_BRACKET 
%token L_SBRACKET 
%token R_SBRACKET 
%token L_PAREN 
%token R_PARENR 
%token SEMICOL 
%token COMMA 
%token COL 
%token DOUBLE_COL 
%token DOT 
%token DOUBLE_DOT 

%token SPACE 
%token NEW_LINE 
%token COMMENT1 
%token COMMENT2
%token OTHER

%right INSERT
%left OR
%left AND
%nonassoc EQUAL NOT_EQUAL
%nonassoc GREATER GREATER_EQUAL LESS LESS_EQUAL 
%right MINUS
%left PLUS
%left MULT DIV MOD
%right NOT PLUS_ONE MINUS_ONE UMINUS
%left DOT DOUBLE_DOT
%left L_SBRACKET R_SBRACKET
%left L_PAREN R_PARENR

%%
program:	stmts {printf("start\n");} 
			|
			;

stmt:	 expr SEMICOL		{printf("%d: stmt-> expr;\n", yylineno);}
		|ifstmt				{printf("%d: stmt-> ifstmt\n", yylineno);}
		|whilestmt 			{printf("%d: stmt-> whilestmt\n", yylineno);}
		|forstmt 			{printf("%d: stmt-> forstmt\n", yylineno);}
		|returnstmt			{printf("%d: stmt-> returnstmt\n", yylineno);}
		|BREAK SEMICOL 		{printf("%d: stmt-> break;\n", yylineno);}
		|CONTINUE SEMICOL	{printf("%d: stmt-> cont;\n", yylineno);}
		|block 				{printf("%d: stmt-> block\n", yylineno);}
		|funcdef 			{printf("%d: stmt-> funcdef\n", yylineno);}
		|SEMICOL 			{printf("%d: stmt-> ;\n", yylineno);}
		;
		
stmts: 	stmt stmts			{printf("%d: stmts-> stmt stmts\n", yylineno);}
		|stmt  				{printf("%d: stmts-> stmt\n", yylineno);}
		;

expr:	term 				{printf("%d: expr-> term\n", yylineno);}
		|values op values	{printf("%d: expr-> val op val\n", yylineno);}
		|lvalue {
			if(LookupInFuncNames(yylval.strVal, scope) == 1){
				printf("Error: can't assign in a function...\n");
			}
		}
		 INSERT expr	{printf("%d: expr-> lval = expr\n", yylineno);}
		;

values: lvalue				{printf("%d: values-> lval\n", yylineno);}
		|const				{printf("%d: values-> const\n", yylineno);}
		;

exprs:	 expr exprss 		{printf("%d: exprs-> expr exprss\n", yylineno);}
		|expr 				{printf("%d: exprs-> expr\n", yylineno);}
		;

op:		OR					{printf("%d: op-> OR\n", yylineno);}
		|AND				{printf("%d: op-> AND\n", yylineno);}
		|EQUAL				{printf("%d: op-> EQUAL\n", yylineno);}
		|NOT_EQUAL			{printf("%d: op-> NOT_EQUAL\n", yylineno);}
		|GREATER			{printf("%d: op-> GREATER\n", yylineno);}
		|GREATER_EQUAL		{printf("%d: op-> GREATER_EQUAL\n", yylineno);}
		|LESS				{printf("%d: op-> LESS\n", yylineno);}
		|LESS_EQUAL			{printf("%d: op-> LESS_EQUAL\n", yylineno);}
		|PLUS				{printf("%d: op-> PLUS\n", yylineno);}
		|MINUS				{printf("%d: op-> MINUS\n", yylineno);}
		|MULT				{printf("%d: op-> MULT\n", yylineno);}
		|DIV				{printf("%d: op-> DIV\n", yylineno);}
		|MOD				{printf("%d: op-> MOD\n", yylineno);}
		;
	

exprss:  COMMA expr exprss 		{printf("%d: exprss-> , expr expr\n", yylineno);}
		|COMMA expr				{printf("%d: exprss-> , expr\n", yylineno);}
		;	

term:	 '-' expr %prec UMINUS			{printf("%d: term-> UMINUS  expr\n", yylineno);}
		|NOT expr 						{printf("%d: term-> NOT expr\n", yylineno);}
		|primary 						{printf("%d: term-> primary\n", yylineno);}
		|lvalue PLUS_ONE				{printf("%d: term-> lvalue++\n", yylineno);
											if(LookupInFuncNames(yylval.strVal, scope) == 1){
												printf("Error: can't plus one a function...\n");
											}

										}
		|PLUS_ONE lvalue 				{printf("%d: term-> ++lvalue\n", yylineno);
											if(LookupInFuncNames(yylval.strVal, scope) == 1){
												printf("Error: can't plus one a function...\n");
											}
										}
		|MINUS_ONE lvalue 				{printf("%d: term-> --lvalue\n", yylineno);
											if(LookupInFuncNames(yylval.strVal, scope) == 1){
												printf("Error: can't minus one a function...\n");
											}
										}
		|lvalue MINUS_ONE				{printf("%d: term-> lvalue--\n", yylineno);
											if(LookupInFuncNames(yylval.strVal, scope) == 1){
												printf("Error: can't minus one a function...\n");
											}
										}
		|L_PAREN expr R_PARENR			{printf("%d: term-> ( expr )\n", yylineno);}
		;

primary:	 lvalue						{printf("%d: primary-> lval\n", yylineno);}
			|call 						{printf("%d: primary-> call\n", yylineno);}
			|objectdef 					{printf("%d: primary-> objectdef\n", yylineno);}
			|const 						{printf("%d: primary-> ( cost )\n", yylineno);}
			|L_PAREN funcdef R_PARENR 	{printf("%d: primary-> ( functdef )\n", yylineno);}
			;

lvalue:	 member 						{printf("%d: lvalue-> member\n", yylineno);}
		|ID 							{printf("%d: lvalue-> id\n", yylineno);
											if(scope == 0){
												if(LookupInScopeVar(scope, yylval.strVal, "var") == 0){
													InsertVar(hashTable, ++key, yylval.strVal, 0, yylineno, 0);
												}
											}
											else{
												if(in_func_block_flag == 1){
													if(LookupInScopeVar(0, yylval.strVal, "var") == 0){
														if(Lookup("var", yylval.strVal, scope-1) == 1){
															printf("Error: Cannot access var %s in line %d \n", yylval.strVal, yylineno);
														}
														else{
														if(LookupInScopeVar(scope, yylval.strVal, "var") == 0){
															InsertVar(hashTable, ++key, yylval.strVal, scope, yylineno, 1);
														}
													}
													}
												}
												else{
													if(Lookup("var", yylval.strVal, scope) == 0){
														InsertVar(hashTable, ++key, yylval.strVal, scope, yylineno, 1);
													}
												}
											}
										}
		|LOCALVAR ID 					{printf("%d: lvalue-> local id\n", yylineno);
										if(LookupInScopeVar(scope, yylval.strVal, "local") == 0){
											InsertVar(hashTable, ++key, yylval.strVal, scope, yylineno, 1);
										}
										}
		|DOUBLE_COL ID 					{printf("%d: lvalue-> double col id\n", yylineno);
										if(LookupInScopeVar(0, yylval.strVal, "var") == 0){
											printf("Error. ::%s not defined.",yylval.strVal);
										}
										}
		;
		
member:	 memvals DOT memvals				{printf("%d: member-> memvlas.id\n", yylineno);}
		|call DOT ID 						{printf("%d: member-> call.id\n", yylineno);}
		|memvals L_SBRACKET expr R_SBRACKET {printf("%d: member-> memvals [ expr ]\n", yylineno);}
		|call L_SBRACKET expr R_SBRACKET 	{printf("%d: member-> call [ expr ]\n", yylineno);}
		;


memvals:  ID 							{printf("%d: memvals-> id\n", yylineno);}
		|LOCALVAR ID 					{printf("%d: memvals-> local id\n", yylineno);}
		|DOUBLE_COL ID 					{printf("%d: memvals-> double col id\n", yylineno);}
		;


		
call:	 lvalue callsuffix	 													{printf("%d: call-> lval callsuffix\n", yylineno);}
		|L_PAREN funcdef R_PARENR L_PAREN elist R_PARENR						{printf("%d: call-> ( funcdef ) ( elist )\n", yylineno);}
		|lvalue callsuffix L_PAREN elist R_PARENR	 							{printf("%d: call-> lval callsuffix ( elist )\n", yylineno);}
		|L_PAREN funcdef R_PARENR L_PAREN elist R_PARENR L_PAREN elist R_PARENR	{printf("%d: call-> ( funcdef ) ( elist ) ( elist )\n", yylineno);}
		;


callsuffix:	 normcall 				{printf("%d: normcall\n", yylineno);}
			|methodcall				{printf("%d: methodcall\n", yylineno);}
			;
			
normcall:	 L_PAREN elistlist R_PARENR	{printf("%d: normcall-> ( elist )\n", yylineno);}
			;
			
methodcall:  DOUBLE_DOT ID L_PAREN elistlist R_PARENR 	{printf("%d: methodcall-> ..id ( elist )\n", yylineno);}
			;

elistlist: elist						{printf("%d: elistlist-> elist\n", yylineno);}
			|							{printf("%d: elistlist-> no elistlist\n", yylineno);}
			;


elist:	 expr							{printf("%d: elist-> expr\n", yylineno);}
		|expr elists					{printf("%d: elist-> expr elists\n", yylineno);}
		;

elists:  COMMA expr elists				{printf("%d: elists-> COMMA expr elists\n", yylineno);}
		|COMMA expr 					{printf("%d: elists-> COMMA expr \n", yylineno);}
		;

objectdef:	 L_SBRACKET R_SBRACKET				{printf("%d: objectdef-> [ ]\n", yylineno);}
			|L_SBRACKET elist R_SBRACKET		{printf("%d: objectdef-> [ elist ]\n", yylineno);}
			|L_SBRACKET indexed R_SBRACKET 		{printf("%d: objectdef-> [ indexed ]\n", yylineno);}
			;
			
indexed:	indexedelems						{printf("%d: indexed-> indexedelems\n", yylineno);}
			;
			
indexedelem:	 L_BRACKET expr COL expr R_BRACKET	{printf("%d: indexedelem-> { expr : expr }\n", yylineno);}
				;
				
indexedelems:	 indexedelems indexedelem		{printf("%d: indexedelems-> indexedelems indexedelem\n", yylineno);}
				|indexedelem					{printf("%d: indexedelems-> indexedelem\n", yylineno);}
				;
				
indexedelemss:	 COMMA indexedelem indexedelemss	{printf("%d: indexedelemss-> , indexedelems indexedelem\n", yylineno);}
				|COMMA indexedelem 					{printf("%d: indexedelemss-> , indexedelem\n", yylineno);}
				;
				
block:	 L_BRACKET {scope++; NewScope(scope); } stmts {Hide (scope);} R_BRACKET	{  scope--; printf("%d: block-> { stmts }\n", yylineno);}
		|L_BRACKET {scope++;Hide (scope);scope--;} R_BRACKET					{printf("%d: block-> { }\n", yylineno);}
		;

funcdef:	 FUNCTION ID {
							funcname= (char*) malloc(sizeof(yylval.strVal));
							funcname = yylval.strVal; 
							if(LookUpinScopeFunc(scope, funcname, "func") == 0){
								InsertFunc(hashTable, ++key, funcname, scope, yylineno, 0, funcargs);
								AddFuncname (funcname, scope);

							}
							else{
								printf("Error: Function with this name already exists.\n");
							}

						} 
				L_PAREN {
						scope++; NewScope(scope); 
					
						} 
				idlist R_PARENR  {
									in_func_block_flag = 1; 
									ResetArgs();
									scope--;
									
								} 
				block {
						
						printf("%d: funcdef-> function id ( idlist ) block\n", yylineno);

						  in_func_block_flag = 0;
						}

			|FUNCTION {
						funcname= (char*) malloc(sizeof(char *)*10); 
						sprintf(funcname, "$f%d", funcnamenum); 
						InsertFunc(hashTable, ++key, funcname, scope, yylineno, 0, funcargs);
						AddFuncname (funcname, scope);
						funcnamenum++; 
						
					} 
			L_PAREN{scope++; NewScope(scope);} 
			idlist R_PARENR {
							in_func_block_flag = 1; 
							ResetArgs();
							scope--; 
							}
			block 			{printf("%d: funcdef-> function ( idlist ) block\n", yylineno); in_func_block_flag = 0;}
			;

const:	 INT 			{printf("%d: const-> int\n", yylineno);}
		|REAL			{printf("%d: const-> real\n", yylineno);}
		|STRING 		{printf("%d: const-> string\n", yylineno);}
		|NIL			{printf("%d: const-> nil\n", yylineno);}
		|TRUE			{printf("%d: const-> true\n", yylineno);}
		|FALSE			{printf("%d: const-> false\n", yylineno);}
		;

idlist:	ids 		{printf("%d: idlist-> ids\n", yylineno);}
		|			{printf("%d: idlist-> no idlist\n", yylineno);}
		;
		
ids:	 ID			{AddArguement(yylval.strVal, scope);}
			idss {printf("%d: ids-> id idss\n", yylineno);}
		|ID			{printf("%d: ids-> id\n", yylineno); AddArguement(yylval.strVal, scope); }

		;
		
idss:	 COMMA ID 		{ AddArguement(yylval.strVal, scope); }
		idss		{printf("%d: idss-> , id idss\n", yylineno);}
		|COMMA ID			{printf("%d: idss-> , id\n", yylineno); AddArguement(yylval.strVal, scope); }
		;

ifstmt:	 IF L_PAREN expr R_PARENR stmt				{printf("%d: ifstmt-> if ( expr )\n", yylineno);}
		|IF L_PAREN expr R_PARENR stmt ELSE stmt	{printf("%d: ifstmt-> if ( expr ) smt else stmt\n", yylineno);}
		;

whilestmt:	WHILE L_PAREN expr R_PARENR stmt 	{printf("%d: whilestmt-> while ( expr ) stmt\n", yylineno);}
			;

forstmt:	FOR L_PAREN elist SEMICOL expr SEMICOL elist R_PARENR stmt	{printf("%d: whilestmt-> forstmt-> for ( elist; expr; elist ) stmt\n", yylineno);}
			;

returnstmt:	 RETURN SEMICOL 			{printf("%d: returnstmt-> return;\n", yylineno);}
			|RETURN expr SEMICOL 		{printf("%d: returnstmt-> return expr;\n", yylineno);}
			;
			

%%
void ResetArgs(){
	funcargs = NULL;
	funcargs = (FuncArgs*) malloc(sizeof(FuncArgs));
	funcargs->arg = NULL;
	funcargs->next = NULL;
}
void AddArguement (char* name,int scope){	
	if(LookupInScopeVar(scope, yylval.strVal, "var") == 0){
		FuncArgs *node;
		int flag = 0;
		node = (FuncArgs*) malloc(sizeof(FuncArgs));
		node->arg = name;
		node->next = NULL;
		if(funcargs->arg == NULL){
			funcargs = node;
			InsertVar(hashTable, key++, name, scope, yylineno, 2);
		}
		else{
			FuncArgs *tmp = funcargs;
			while(tmp->next != NULL){
				if(tmp->arg == name){
				printf("Error: Arguement with the name %s already exists.\n", name);
				flag = 1;
				break;
			}
				tmp = tmp->next;
			}
			if(flag == 0){
				InsertVar(hashTable, key++, name, scope, yylineno, 2);
				tmp->next = node;
			} 
		}
	}

}

AddFuncname (char *name, int scope){

	int flag = 0;
	FuncNames *node;
	node = (FuncNames*) malloc(sizeof(FuncNames));
	node->name = name;
	node->scope = scope;
	node->next = NULL;

	if(funcnames->name == NULL){
		funcnames = node;
	}
	else{
		FuncNames *tmp = funcnames;
		while(tmp->next != NULL){
			tmp = tmp-> next;
		}
		tmp->next = node;
		
	}
}

int LookupInFuncNames (char *name, int scope){
	FuncNames *tmp;
	tmp = funcnames;

	if(tmp->name == NULL){
		return 0;
	}
	while (tmp != NULL){
		if(strcmp(tmp->name, name) == 0 && scope>= tmp->scope){
			return 1;
		}
		tmp = tmp->next;
	}
	return 0;
}


void yyerror(void* a,char* yaccProvidedMessasge){
	fprintf(stderr, "%s: at line %d, before token: %s\n", yaccProvidedMessasge, yylineno, yytext);
	fprintf(stderr, "INPUT NOT VALID\n");
}

void Init(){
	
	hashTable = SymTable_new();
	scopesHeadsList= (ScopesHeadsList*)malloc(sizeof(ScopesHeadsList));
	scopesHeadsList -> next = NULL;
	scopesHeadsList->scopeHead =  NULL;
	funcargs = (FuncArgs*)malloc(sizeof(FuncArgs));
	funcargs->arg = NULL;
	funcargs->next = NULL;

	funcnames = (FuncNames*)malloc(sizeof(FuncNames));
	funcnames->name = NULL;
	funcnames->next = NULL;

	InsertFunc(hashTable, key++, "print", 0, 0, 1, funcargs);
	InsertFunc(hashTable, key++, "input", 0, 0, 1, funcargs);
	InsertFunc(hashTable, key++, "objectmemberkeys", 0, 0, 1, funcargs);
	InsertFunc(hashTable, key++, "objecttotalmembers", 0, 0, 1, funcargs);
	InsertFunc(hashTable, key++, "objectcopy", 0, 0, 1, funcargs);
	InsertFunc(hashTable, key++, "totalarguments", 0, 0, 1, funcargs);
	InsertFunc(hashTable, key++, "argument", 0, 0, 1, funcargs);
	InsertFunc(hashTable, key++, "typeof", 0, 0, 1, funcargs);
	InsertFunc(hashTable, key++, "strtonum", 0, 0, 1, funcargs);
	InsertFunc(hashTable, key++, "sqrt", 0, 0, 1, funcargs);
	InsertFunc(hashTable, key++, "cos", 0, 0, 1, funcargs);
	InsertFunc(hashTable, key++, "sin", 0, 0, 1, funcargs);

}

int main(int argc, char** argv){
	if(argc > 1){
		if (!(yyin = fopen(argv[1], "r"))){
			fprintf(stderr, "Cannot read file: %s\n", argv[1]);
			return 1;
		}	
	}else{
		yyin = stdin;
	}
	Init();
	yyparse(NULL);
	PrintScopes();
	//PrintHash(hashTable);
	return 0;
}

//Επίλογος (προαιρετικό)