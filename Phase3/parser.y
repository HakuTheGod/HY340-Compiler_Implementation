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

	SymbolTableEntry* sym;
	SymbolTable *hashTable;
	offsets* offSets, *tailOffSet;
	int flagError;
	int loopcounter = 0;
	int in_func_block_flag= 0;
	int bool_flag = 0;
	int bool_op_flag = 0;
	int memFlag = 0;
	int key=-1, scope=0;
	struct expr* expression;
	int prog_var_offset=0, func_locals_offset=0, formal_args_offset=0;
	char* hidden_val;
	FuncArgs *funcargs;
	char *funcname;
	char* memlval;
	struct expr* arg1;
	struct expr* arg2;
	int funcnamenum=0;
	FuncNames *funcnames;
	struct expr* lval_for_call;
	char* call_name;
	struct expr* elists_heads;
	struct expr* elists_tail;
	struct calls* for_method;
	struct loopcounter_t* loophead;
	struct table_items* table_head;
	int bool_id_flag = 0;

	struct expr* true_list;
	struct expr* false_list;

	void Rmoffset();
	void AddOffset(int localvaroffset);
	void AddArguement(char* name, int scope);
	void ResetArgs();
	void Init();
	void AddElist (struct  expr* node);
	void Backpatch (struct true_false* list, unsigned M);
	void push_head_elist();
	void push (struct loopcounter_t* stack, int loopcounter);
	int pop (struct loopcounter_t* stack);
	void PushElist(struct expr* node);
	void PopTable ();
	void PushTable (struct expr* index, struct expr* val);
	void ReplaceQuads (int a, int b);

	struct true_false* merge (struct true_false* list1, struct true_false* list2);

	typeof (struct calls*) callsuffix;
	typeof(struct calls*) normcall;
	typeof(struct calls*) methodcall;
	typeof(struct table_items) indexed ;
	typeof(struct table_items) indexedelems ;
	typeof(struct expr*) lvalue ;
	typeof(struct expr*) primary ;
	typeof(struct expr*) expr ;
	typeof(struct expr*) assginexpr ;
	typeof(struct expr*) member ;
	typeof(struct expr*) elist ;
	typeof(struct expr*) elistlist ;
	typeof(struct expr*) term ;
	typeof(struct expr*) objectdef ;
	typeof(struct expr*) funcprefix ;
	typeof(struct expr*) funcdef ;
	typeof(struct expr*) constant;
	typeof(char*) funcname;
	typeof(int) funcbody;
	typeof(int) ifprefix;
	typeof(int) elseprefix;
	typeof(int) L;
	typeof(int) N;
	typeof(int) F;
	typeof(int) whilestart;
	typeof(int) whilecond;
	typeof(struct stmt_t*) stmt;
	typeof(struct stmt_t*) stmts;
	typeof(struct stmt_t*) loopstmt;
	typeof(struct stmt_t*) block;
	typeof(struct stmt_t*) ifstmt;
	typeof(struct enter_test*) forprefix;

%}
%lex-param { void* val }
%parse-param { void* val }
%union {
	struct SymbolTableEntry* s;
	struct enter_test* enter;
	struct stmt_t* stmtt;
	struct expr* e;
	char* strVal;
	int intVal;
	double realVal;
	struct calls*  call;
	struct table_items* tableitems;
}

%start program
%type <e> constant
%type <e> lvalue
%type <e> primary
%type <e> expr
%token <e> ID
%type <e> call
%type <e> assginexpr
%type <e> member
%type <call> methodcall
%type <call> normcall
%type <call> callsuffix
%type <e> elist
%type <e> elistlist
%type <e> term
%type <e> objectdef
%token <intVal> INT
%type <strVal> funcname
%type <intVal> funcbody
%type <e> funcprefix
%type <e> funcdef
%type <intVal> M
%type <intVal> ifprefix
%type <intVal> elseprefix
%type <enter> forprefix
%type <intVal> L
%type <intVal> N
%type <intVal> F
%type <intVal> whilestart
%type <intVal> whilecond
%type <stmtt> stmt
%type <stmtt> loopstmt
%token <stmtt> BREAK
%token <stmtt> CONTINUE
%type <stmtt> stmts
%type <stmtt> block
%token <e> NIL
%token <e> TRUE
%token <e> FALSE
%type <tableitems> indexed
%type <tableitems> indexedelems
%type <stmtt> ifstmt



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
%token UMINUS

%token <realVal> REAL 
%token <strVal> STRING 

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
%left PLUS MINUS
%left MULT DIV MOD
%right NOT PLUS_ONE MINUS_ONE UMINUS
%left DOT DOUBLE_DOT
%left L_SBRACKET R_SBRACKET
%left L_PAREN R_PARENR



%%
program:	stmts {printf("start\n"); } 
			|
			;

stmt:	 expr SEMICOL		{printf("%d: stmt-> expr;\n", yylineno);
								$stmt = make_stmt();
								bool_flag = 0;
								
								bool_op_flag = 0;
								resettemp();
							}
		|ifstmt				{printf("%d: stmt-> ifstmt\n", yylineno);
								$stmt = $ifstmt;

								resettemp();
							}
		|whilestmt 			{printf("%d: stmt-> whilestmt\n", yylineno);
								$stmt = make_stmt();
							resettemp();
							}
		|forstmt 			{printf("%d: stmt-> forstmt\n", yylineno);
								$stmt = make_stmt();
							resettemp();
							}
		|returnstmt			{printf("%d: stmt-> returnstmt\n", yylineno);
								$stmt = make_stmt();
							resettemp();
							}
		|BREAK SEMICOL 		{printf("%d: stmt-> break;\n", yylineno);
							$stmt = make_stmt();
							newlist($stmt->breaklist);
							emit(jump, NULL, NULL, NULL, currQuad, yylineno);
							resettemp();
							}
		|CONTINUE SEMICOL	{printf("%d: stmt-> cont;\n", yylineno);
								$stmt = make_stmt();
								newlist ($stmt->contlist);
								emit(jump, NULL, NULL, NULL, currQuad, yylineno);
								resettemp();
							}
		|block 				{printf("%d: stmt-> block\n", yylineno);
								
								$stmt = $block;
							resettemp();
							}
		|funcdef 			{printf("%d: stmt-> funcdef\n", yylineno);
								$stmt = make_stmt();
							resettemp();
							}
		|SEMICOL 			{printf("%d: stmt-> ;\n", yylineno);
								$stmt = make_stmt();
							resettemp();
							}
		| error SEMICOL {yyerrok;}
		;
		
stmts: 	stmt stmts			{printf("%d: stmts-> stmt stmts\n", yylineno);
								$$ = make_stmt();

								if(($2 != NULL) || ($1 != NULL)){
									$$->breaklist = mergelist ($1->breaklist, $2->breaklist);
									$$->contlist = mergelist ($1->contlist, $2->contlist);
								}  
								
							}
		|stmt  				{printf("%d: stmts-> stmt\n", yylineno);
								$stmts = $stmt;
							}
		;

expr:	assginexpr				{printf("%d: expr-> assginexpr\n", yylineno);
									$$ = $1;
									bool_id_flag = 1;
								}
		|expr PLUS expr			{printf("%d: op-> PLUS\n", yylineno);

									if($1->type == constnum_e && $3->type == constnum_e){
										$$ = newexpr(constnum_e);
										$$->sym = newtemp();
										if($1->numConst != 0){
											if ($3->numConst != 0){
												$$->numConst = $1->numConst +$3->numConst;
											}
											else{
												$$->numConst = $1->numConst +$3->intConst;
											}
										}
										else if($3->numConst != 0){
											$$->numConst = $1->intConst +$3->numConst;
										}
										else{
											$$->intConst = $1->intConst + $3->intConst;
										}
										prog_var_offset++;
										emit(add,$1,$3,$$,currQuad,yylineno);
									}

									else if(($1->type != programfunc_e) && ($1->type != libraryfunc_e) && ($1->type != boolexpr_e) && ($1->type != newtable_e) && ($1->type != constbool_e) && ($1->type != conststring_e) && ($1->type != nil_e)){
										if($3->type != programfunc_e && $3->type != libraryfunc_e && $3->type != boolexpr_e && $3->type != newtable_e && $3->type != constbool_e && $3->type != conststring_e && $3->type != nil_e){
											$$ = newexpr(arithexpr_e);
											$$->sym = newtemp();
											prog_var_offset++;
											emit(add,$1,$3,$$,currQuad,yylineno);

										}
									}
									else{
										printf("Error: expressions in add are of incompatible type.\n");
									}

								}
		|expr MINUS	expr		{printf("%d: op-> MINUS\n", yylineno);
									if($1->type == constnum_e && $3->type == constnum_e){
										$$ = newexpr(constnum_e);
										$$->sym = newtemp();
										if($1->numConst != 0){
											if ($3->numConst != 0){
												$$->numConst = $1->numConst - $3->numConst;
											}
											else{
												$$->numConst = $1->numConst - $3->intConst;
											}
										}
										else if($3->numConst != 0){
											$$->numConst = $1->intConst - $3->numConst;
										}
										else{
											$$->intConst = $1->intConst - $3->intConst;
										}
										prog_var_offset++;
										emit(sub,$1,$3,$$,currQuad,yylineno);
									}

									else if(($1->type != programfunc_e) && ($1->type != libraryfunc_e) && ($1->type != boolexpr_e) && ($1->type != newtable_e) && ($1->type != constbool_e) && ($1->type != conststring_e) && ($1->type != nil_e)){
										if($3->type != programfunc_e && $3->type != libraryfunc_e && $3->type != boolexpr_e && $3->type != newtable_e && $3->type != constbool_e && $3->type != conststring_e && $3->type != nil_e){
											$$ = newexpr(arithexpr_e);
											$$->sym = newtemp();
											prog_var_offset++;
											emit(sub,$1,$3,$$,currQuad,yylineno);
										}
									}
									else{
										printf("Error: expressions in add are of incompatible type.\n");
									}
									
								}
		|expr MULT expr			{printf("%d: op-> MULT\n", yylineno);
									if($1->type == constnum_e && $3->type == constnum_e){
										$$ = newexpr(constnum_e);
										$$->sym = newtemp();
										if($1->numConst != 0){
											if ($3->numConst != 0){
												$$->numConst = $1->numConst * $3->numConst;
											}
											else{
												$$->numConst = $1->numConst * $3->intConst;
											}
										}
										else if($3->numConst != 0){
											$$->numConst = $1->intConst * $3->numConst;
										}
										else{
											$$->intConst = $1->intConst * $3->intConst;
										}
										prog_var_offset++;
										emit(mul,$1,$3,$$,currQuad,yylineno);
									}

									else if(($1->type != programfunc_e) && ($1->type != libraryfunc_e) && ($1->type != boolexpr_e) && ($1->type != newtable_e) && ($1->type != constbool_e) && ($1->type != conststring_e) && ($1->type != nil_e)){
										if($3->type != programfunc_e && $3->type != libraryfunc_e && $3->type != boolexpr_e && $3->type != newtable_e && $3->type != constbool_e && $3->type != conststring_e && $3->type != nil_e){
											$$ = newexpr(arithexpr_e);
											$$->sym = newtemp();
											prog_var_offset++;
											emit(mul,$1,$3,$$,currQuad,yylineno);
										}
									}
									else{
										printf("Error: expressions in add are of incompatible type.\n");
									}
								}
		|expr DIV expr			{printf("%d: op-> DIV\n", yylineno);
									if($1->type == constnum_e && $3->type == constnum_e){
										$$ = newexpr(constnum_e);
										$$->sym = newtemp();
										if($1->numConst != 0){
											if ($3->numConst != 0){
												$$->numConst = $1->numConst / $3->numConst;
											}
											else{
												$$->numConst = $1->numConst / $3->intConst;
											}
										}
										else if($3->numConst != 0){
											$$->numConst = $1->intConst / $3->numConst;
										}
										else if ($3->numConst != 0){
											$$->intConst = $1->intConst / $3->intConst;
										}
										else printf("Error: cannot divide by 0.\n");
										prog_var_offset++;
										emit(divv,$1,$3,$$,currQuad,yylineno);
									}

									else if(($1->type != programfunc_e) && ($1->type != libraryfunc_e) && ($1->type != boolexpr_e) && ($1->type != newtable_e) && ($1->type != constbool_e) && ($1->type != conststring_e) && ($1->type != nil_e)){
										if($3->type != programfunc_e && $3->type != libraryfunc_e && $3->type != boolexpr_e && $3->type != newtable_e && $3->type != constbool_e && $3->type != conststring_e && $3->type != nil_e){
											$$ = newexpr(arithexpr_e);
											$$->sym = newtemp();
											prog_var_offset++;
											emit(divv,$1,$3,$$,currQuad,yylineno);
										}
									}
									else{
										printf("Error: expressions in add are of incompatible type.\n");
									}
								}
		|expr MOD expr			{printf("%d: op-> MOD\n", yylineno);
									if($1->type == constnum_e && $3->type == constnum_e){
										$$ = newexpr(constnum_e);
										$$->sym = newtemp();
										if($1->numConst == 0 && $3->numConst == 0){
											$$->intConst = $1->intConst % $3->intConst;
										}
										prog_var_offset++;
										emit(mod,$1,$3,$$,currQuad,yylineno);
									}

									else if(($1->type != programfunc_e) && ($1->type != libraryfunc_e) && ($1->type != boolexpr_e) && ($1->type != newtable_e) && ($1->type != constbool_e) && ($1->type != conststring_e) && ($1->type != nil_e)){
										if($3->type != programfunc_e && $3->type != libraryfunc_e && $3->type != boolexpr_e && $3->type != newtable_e && $3->type != constbool_e && $3->type != conststring_e && $3->type != nil_e){
											$$ = newexpr(arithexpr_e);
											$$->sym = newtemp();
											prog_var_offset++;
											emit(mod,$1,$3,$$,currQuad,yylineno);
										}
									}
									else{
										printf("Error: expressions in add are of incompatible type.\n");
									}
									
								}	
		|expr GREATER expr			{printf("%d: op-> GREATER\n", yylineno);
		printf("In greater = bool = %d, bool id = %d, bool op = %d\n", bool_flag, bool_id_flag, bool_op_flag);
										if(bool_flag == 1 && bool_id_flag == 0){
											currQuad = currQuad - 3;
										}
										
										$$ = newexpr(boolexpr_e);
										$$->sym = newtemp();

										struct expr* true_expr = newexpr_constbool('T');
										struct expr* false_expr = newexpr_constbool('F');

										
										$$->true_list->index = currQuad;
										$$->false_list->index = currQuad + 1;

										emit(if_greater, $1, $3, NULL, currQuad, yylineno);

										emit(jump, NULL, NULL, NULL, currQuad, yylineno);

										struct expr* jump_int = newexpr(jump);
										jump_int->intConst = currQuad + 3;

										Backpatch ($$->true_list, currQuad);
										emit(assign, true_expr, NULL, $$, currQuad, yylineno);
										emit(jump, NULL, NULL, jump_int, currQuad, yylineno);
										Backpatch ($$->false_list, currQuad);
										emit(assign, false_expr, NULL, $$, currQuad, yylineno);
										bool_flag = 1;
										bool_id_flag = 0;
										bool_op_flag = 1;
										
										
									}
		|expr GREATER_EQUAL	expr	{printf("%d: op-> GREATER_EQUAL\n", yylineno);
		printf("In greater = bool = %d, bool id = %d, bool op = %d\n", bool_flag, bool_id_flag, bool_op_flag);
										if(bool_flag == 1 && bool_id_flag == 0) currQuad = currQuad - 3;
										$$ = newexpr(boolexpr_e);
										$$->sym = newtemp();

										$$->true_list->index = currQuad;
										$$->false_list->index = currQuad + 1;

										emit(if_greatereq, $1, $3, NULL, currQuad, yylineno);

										emit(jump, NULL, NULL, NULL, currQuad, yylineno);

										struct expr* true_expr = newexpr_constbool('T');
										struct expr* false_expr = newexpr_constbool('F');


										struct expr* jump_int = newexpr(jump);
										jump_int->intConst = currQuad + 3;

										Backpatch ($$->true_list, currQuad);
										emit(assign, true_expr, NULL, $$, currQuad, yylineno);
										emit(jump, NULL, NULL, jump_int, currQuad, yylineno);
										Backpatch ($$->false_list, currQuad);
										emit(assign, false_expr, NULL, $$, currQuad, yylineno);
										bool_flag = 1;
										bool_id_flag = 0;
										bool_op_flag = 1;
									}
		|expr LESS expr				{printf("%d: op-> LESS\n", yylineno);
		printf("In less = bool = %d, bool id = %d, bool op = %d\n", bool_flag, bool_id_flag, bool_op_flag);
										
										if(bool_flag == 1 && bool_id_flag == 0)
										currQuad = currQuad - 3;
										$$ = newexpr(boolexpr_e);
										$$->sym = newtemp();

										$$->true_list->index = currQuad;
										$$->false_list->index = currQuad + 1;
										
										
										emit(if_less, $1, $3, NULL, currQuad, yylineno);
										emit(jump, NULL, NULL, NULL, currQuad, yylineno);


										struct expr* true_expr = newexpr_constbool('T');
										struct expr* false_expr = newexpr_constbool('F');



										struct expr* jump_int = newexpr(jump);
										jump_int->intConst = currQuad + 3;

										Backpatch ($$->true_list, currQuad);
										emit(assign, true_expr, NULL, $$, currQuad, yylineno);
										emit(jump, NULL, NULL, jump_int, currQuad, yylineno);
										Backpatch ($$->false_list, currQuad);
										emit(assign, false_expr, NULL, $$, currQuad, yylineno);
										bool_flag = 1;
										bool_id_flag = 0;
										bool_op_flag = 1;
									}
		|expr LESS_EQUAL expr		{printf("%d: op-> LESS_EQUAL\n", yylineno);
		printf("In lesseq = bool = %d, bool id = %d, bool op = %d\n", bool_flag, bool_id_flag, bool_op_flag);
										if(bool_flag == 1 && bool_id_flag == 0)
											currQuad = currQuad - 3;
										$$ = newexpr(boolexpr_e);
										$$->sym = newtemp();

										$$->true_list->index = currQuad;
										$$->false_list->index = currQuad + 1;

										emit(if_lesseq, $1, $3, NULL, currQuad, yylineno);
										emit(jump, NULL, NULL, NULL, currQuad, yylineno);

										struct expr* true_expr = newexpr_constbool('T');
										struct expr* false_expr = newexpr_constbool('F');


										struct expr* jump_int = newexpr(jump);
										jump_int->intConst = currQuad + 3;

										Backpatch ($$->true_list, currQuad);
										emit(assign, true_expr, NULL, $$, currQuad, yylineno);
										emit(jump, NULL, NULL, jump_int, currQuad, yylineno);
										Backpatch ($$->false_list, currQuad);
										emit(assign, false_expr, NULL, $$, currQuad, yylineno);
										bool_flag = 1;
										bool_id_flag = 0;
										bool_op_flag = 1;
									}
		|expr EQUAL expr			{printf("%d: op-> EQUAL bool = %d, bool id = %d, bool op = %d\n", yylineno, bool_flag, bool_id_flag, bool_op_flag);
										printf("In eq = bool = %d, bool id = %d, bool op = %d\n", bool_flag, bool_id_flag, bool_op_flag);
										if(bool_flag == 1  && bool_id_flag == 0) currQuad = currQuad - 3;

										$$ = newexpr(boolexpr_e);
										$$->sym = newtemp();

										$$->true_list->index = currQuad;
										$$->false_list->index = currQuad + 1;

									
										emit(if_eq, $1, $3, NULL, currQuad, yylineno);
										emit(jump, NULL, NULL, NULL, currQuad, yylineno);

										struct expr* true_expr = newexpr_constbool('T');
										struct expr* false_expr = newexpr_constbool('F');


										struct expr* jump_int = newexpr(jump);
										jump_int->intConst = currQuad + 3;

										Backpatch ($$->true_list, currQuad);
										emit(assign, true_expr, NULL, $$, currQuad, yylineno);
										emit(jump, NULL, NULL, jump_int, currQuad, yylineno);
										Backpatch ($$->false_list, currQuad);
										emit(assign, false_expr, NULL, $$, currQuad, yylineno);
										bool_flag = 1;
										bool_id_flag = 0;
										bool_op_flag = 1;
									}
		|expr NOT_EQUAL expr		{printf("%d: op-> NOT_EQUAL\n", yylineno);
		printf("In noteq = bool = %d, bool id = %d, bool op = %d\n", bool_flag, bool_id_flag, bool_op_flag);
										if(bool_flag == 1 && bool_id_flag == 0) currQuad = currQuad - 3;
										$$ = newexpr(boolexpr_e);
										$$->sym = newtemp();

										$$->true_list->index = currQuad;
										$$->false_list->index = currQuad + 1;

										emit(if_noteq, $1, $3, NULL, currQuad, yylineno);
										emit(jump, NULL, NULL, NULL, currQuad, yylineno);

										struct expr* true_expr = newexpr_constbool('T');
										struct expr* false_expr = newexpr_constbool('F');


										struct expr* jump_int = newexpr(jump);
										jump_int->intConst = currQuad + 3;

										Backpatch ($$->true_list, currQuad);
										emit(assign, true_expr, NULL, $$, currQuad, yylineno);
										emit(jump, NULL, NULL, jump_int, currQuad, yylineno);
										Backpatch ($$->false_list, currQuad);
										emit(assign, false_expr, NULL, $$, currQuad, yylineno);
										bool_flag = 1;
										bool_id_flag = 0;
										bool_op_flag = 1;
									}
		|expr AND M F expr  		{printf("%d: op-> AND curr quad = %d\n", yylineno, currQuad);
		printf("In and = bool = %d, bool id = %d, bool op = %d\n", bool_flag, bool_id_flag, bool_op_flag);
										
										if(bool_flag == 1 && bool_id_flag == 0){
											currQuad = currQuad - 3;
										}

										struct expr* true_expr = newexpr_constbool('T');
										struct expr* false_expr = newexpr_constbool('F');

										printf("F = %d and bool_id_flag = %d \n M = %d", $4, bool_id_flag, $3);

										if($4 == 1){
											printf ("1st expression is id.......................\n");
											$1->false_list = (struct true_false*)malloc(sizeof(struct true_false));
											$1->true_list = (struct true_false*)malloc(sizeof(struct true_false));

											$1->true_list->index = currQuad;
											$1->false_list->index = currQuad + 1;

											emit(if_eq, true_expr, $1, NULL, currQuad, yylineno);

											emit(jump, NULL, NULL, NULL, currQuad, yylineno);
										}
										if(bool_id_flag == 1){
											printf ("2nd expression is id.......................\n");
											$5->false_list = (struct true_false*)malloc(sizeof(struct true_false));
											$5->true_list = (struct true_false*)malloc(sizeof(struct true_false));

											$5->true_list->index = currQuad;
											$5->false_list->index = currQuad + 1;

											emit(if_eq, true_expr, $5, NULL, currQuad, yylineno);

											emit(jump, NULL, NULL, NULL, currQuad, yylineno);
										}

										printf("$1 truelist = %d\n", $1->true_list->index);


										$$ = newexpr(boolexpr_e);
										$$->sym = newtemp();

										printf ("About to Backpatch %d in %d.\n", $M, $1->true_list->index);
										Backpatch($1->true_list, $M);
										$$->true_list = $5->true_list;
										$$->false_list = merge($1->false_list, $5->false_list);


										struct expr* jump_int = newexpr(jump);
										jump_int->intConst = currQuad + 3;

										Backpatch ($$->true_list, currQuad);
										emit(assign, true_expr, NULL, $$, currQuad, yylineno);
										emit(jump, NULL, NULL, jump_int, currQuad, yylineno);
										Backpatch ($$->false_list, currQuad);
										emit(assign, false_expr, NULL, $$, currQuad, yylineno);

										if($4 == 1 ){
											ReplaceQuads(currQuad - 5, currQuad - 7);
										}

										struct true_false* tmp = $$->true_list;

										printf("TRUELIST: %d\n", tmp->index);
										while(tmp->next!=NULL){
											tmp = tmp->next;
											printf("TRUELIST: %d\n", tmp->index);
										}
										tmp = $$->false_list;
										printf("FALSELIST: %d\n", tmp->index);
										while(tmp->next!=NULL){
											tmp = tmp->next;
											printf("FALSELIST: %d\n", tmp->index);
										}

										bool_flag = 1;
										bool_id_flag = 0;
										bool_op_flag = 1;

									}
		|expr OR M F expr				{printf("%d: op-> OR\n", yylineno);
		printf("In or = bool = %d, bool id = %d, bool op = %d\n", bool_flag, bool_id_flag, bool_op_flag);
										if(bool_flag == 1 && bool_id_flag == 0){
											currQuad = currQuad - 3;
										}

										struct expr* true_expr = newexpr_constbool('T');
										struct expr* false_expr = newexpr_constbool('F');

										printf("F = %d and bool_id_flag = %d \n M = %d", $4, bool_id_flag, $3);

										if($4 == 1){
											printf ("1st expression is id.......................\n");
											$1->false_list = (struct true_false*)malloc(sizeof(struct true_false));
											$1->true_list = (struct true_false*)malloc(sizeof(struct true_false));

											$1->true_list->index = currQuad;
											$1->false_list->index = currQuad + 1;

											emit(if_eq, true_expr, $1, NULL, currQuad, yylineno);

											emit(jump, NULL, NULL, NULL, currQuad, yylineno);
										}
										if(bool_id_flag == 1){
											printf ("2nd expression is id.......................\n");
											$5->false_list = (struct true_false*)malloc(sizeof(struct true_false));
											$5->true_list = (struct true_false*)malloc(sizeof(struct true_false));

											$5->true_list->index = currQuad;
											$5->false_list->index = currQuad + 1;

											emit(if_eq, true_expr, $5, NULL, currQuad, yylineno);

											emit(jump, NULL, NULL, NULL, currQuad, yylineno);
										}

										$$ = newexpr(boolexpr_e);
										$$->sym = newtemp();
										
										printf("In or: About to backpatch M = %d in $1 = %d.\n", $M, $1->false_list->index);
										Backpatch($1->false_list, $M);
										
										$$->true_list = merge($1->true_list, $5->true_list);
										$$->false_list = $5->false_list;


										struct expr* jump_int = newexpr(jump);
										jump_int->intConst = currQuad + 3;

										Backpatch ($$->true_list, currQuad);
										emit(assign, true_expr, NULL, $$, currQuad, yylineno);
										emit(jump, NULL, NULL, jump_int, currQuad, yylineno);
										Backpatch ($$->false_list, currQuad);
										emit(assign, false_expr, NULL, $$, currQuad, yylineno);

										if($4 == 1 ){
											ReplaceQuads(currQuad - 5, currQuad - 7);
										}

										struct true_false* tmp = $$->true_list;

										printf("TRUELIST: %d\n", tmp->index);
										while(tmp->next!=NULL){
											tmp = tmp->next;
											printf("TRUELIST: %d\n", tmp->index);
										}
										tmp = $$->false_list;
										printf("FALSELIST: %d\n", tmp->index);
										while(tmp->next!=NULL){
											tmp = tmp->next;
											printf("FALSELIST: %d\n", tmp->index);
										}

										bool_flag = 1;
										bool_id_flag = 0;
										bool_op_flag = 1;
									}
		|term 				{printf("%d: expr-> term\n", yylineno);
							$expr = $term;
							
		}
		;

M:		{if(bool_id_flag == 1) $M = currQuad+2;
		 else $M = currQuad - 3;}
		;

F:		{ printf("In F = bool = %d, bool id = %d, bool op = %d\n", bool_flag, bool_id_flag, bool_op_flag);
	if(bool_op_flag == 1) currQuad = currQuad - 3;
	if(bool_id_flag == 1){
			
			expression->false_list = (struct true_false*)malloc(sizeof(struct true_false));
			expression->true_list = (struct true_false*)malloc(sizeof(struct true_false));
			expression->true_list->index = currQuad;
			expression->false_list->index = currQuad + 1;



			struct expr* true_expr = newexpr_constbool('T');
			// Segmentation.
			emit(if_eq, true_expr, expression, NULL, currQuad, yylineno);
			emit(jump, NULL, NULL, NULL, currQuad, yylineno);

			struct expr* jump_int = newexpr(jump);
			jump_int->intConst = currQuad + 3;
		}
		};

assginexpr:	lvalue {
				if(memFlag == 1){
					if(LookupInFuncNames(memlval, scope) == 1){
						printf("Error: can't assign in a function...\n");
					}
				}else if(LookupInFuncNames(yylval.strVal, scope) == 1){
					printf("Error: can't assign in a function...\n");
				}
			} INSERT expr 	{
								printf("%d: assginexpr-> lvalue INSERT expr\n", yylineno);
								if($lvalue->type == tableitem_e){
									emit(tablesetelem, $lvalue, $lvalue->index, $expr, currQuad, yylineno);
									$assginexpr = emit_iftableitem($lvalue, currQuad, yylineno);
									$assginexpr->type = assignexpr_e;
								} 
								else{
									emit(assign, $expr, NULL, $lvalue, currQuad, yylineno);
									$assginexpr = newexpr(assignexpr_e);
									$assginexpr->sym = newtemp();
									emit(assign, $lvalue, NULL, $assginexpr, currQuad, yylineno);
								}
							}
			;

term:	'-' expr %prec UMINUS 		{printf("%d: term-> UMINUS  expr\n", yylineno);
											check_arith($expr, yylval.strVal);
											$term = newexpr(arithexpr_e);
											$term->sym = newtemp();
											emit(uminus, $expr, NULL, $term, currQuad, yylineno);
										}
		|NOT expr 						{printf("%d: term-> NOT expr\n", yylineno);
		printf("In noteq = bool = %d, bool id = %d, bool op = %d\n", bool_flag, bool_id_flag, bool_op_flag);
											
											if(bool_flag == 1  && bool_id_flag == 0) currQuad = currQuad - 3;
											$$ = newexpr(boolexpr_e);
											$$->sym = newtemp();

											struct expr* true_expr = newexpr_constbool('T');
											struct expr* false_expr = newexpr_constbool('F');

											if(bool_op_flag == 0 ){
												$$->true_list->index = currQuad + 1;
												$$->false_list->index = currQuad;
										
												emit(if_eq, $2, true_expr, NULL, currQuad, yylineno);
												emit(jump, NULL, NULL, NULL, currQuad, yylineno);

											}
											else {
												$$->true_list = $2->false_list;
												$$->false_list = $2->true_list;
											}
	
											
	
	
											struct expr* jump_int = newexpr(jump);
											jump_int->intConst = currQuad + 3;
	
											Backpatch ($$->true_list, currQuad);
											emit(assign, true_expr, NULL, $$, currQuad, yylineno);
											emit(jump, NULL, NULL, jump_int, currQuad, yylineno);
											Backpatch ($$->false_list, currQuad);
											emit(assign, false_expr, NULL, $$, currQuad, yylineno);
											bool_flag = 1;
											bool_op_flag = 1;
											bool_id_flag = 0;

										}
		|primary 						{printf("%d: term-> primary\n", yylineno);
											$term = $primary;
										}
		|lvalue PLUS_ONE				{printf("%d: term-> lvalue++\n", yylineno);
											if(LookupInFuncNames(yylval.strVal, scope) == 1){
												printf("Error: can't plus one a function...\n");
											}else{
												check_arith($lvalue, yylval.strVal);
												$term = newexpr(var_e);
												$term->sym = newtemp();
												if($lvalue->type == tableitem_e){
													struct expr* val = emit_iftableitem($lvalue, currQuad, yylineno);
													emit(assign, val , NULL, $term, currQuad, yylineno);
													emit(add , val , newexpr_constnum(1), val, currQuad, yylineno);
													emit(tablesetelem , $lvalue , $lvalue->index, val, currQuad, yylineno);
												}
												else{
													emit(assign , $lvalue , NULL, $term, currQuad, yylineno);
													emit(add , $lvalue , newexpr_constnum(1), $lvalue, currQuad, yylineno);
												}
											}

										}
		|PLUS_ONE lvalue 				{printf("%d: term-> ++lvalue\n", yylineno);
											if(LookupInFuncNames(yylval.strVal, scope) == 1){
												printf("Error: can't plus one a function...\n");
											}else{
												check_arith($lvalue, yylval.strVal);
												if($lvalue->type == tableitem_e){
													$term = emit_iftableitem ($lvalue, currQuad, yylineno);
													emit(add , $term , newexpr_constnum(1), $term, currQuad, yylineno);
													emit(tablesetelem , $lvalue, $lvalue->index, $term, currQuad, yylineno);
												}else{
													emit(add, $lvalue, newexpr_constnum(1), $lvalue, currQuad, yylineno);
													$term = newexpr(arithexpr_e);
													$term->sym = newtemp();
													emit(assign, $lvalue, NULL, $term, currQuad, yylineno);
												}
											}
										}
		|MINUS_ONE lvalue 				{printf("%d: term-> --lvalue\n", yylineno);
											if(LookupInFuncNames(yylval.strVal, scope) == 1){
												printf("Error: can't minus one a function...\n");
											}else{
												check_arith($lvalue, yylval.strVal);
												if($lvalue->type == tableitem_e){
													$term = emit_iftableitem ($lvalue, currQuad, yylineno);
													emit(sub , $term , newexpr_constnum(1), $term, currQuad, yylineno);
													emit(tablesetelem , $lvalue, $lvalue->index, $term, currQuad, yylineno);
												}else{
													emit(sub, $lvalue, newexpr_constnum(1), $lvalue, currQuad, yylineno);
													$term = newexpr(arithexpr_e);
													$term->sym = newtemp();
													emit(assign, $lvalue, NULL, $term, currQuad, yylineno);
												}
											}
										}
		|lvalue MINUS_ONE				{printf("%d: term-> lvalue--\n", yylineno);
		
											if(LookupInFuncNames(yylval.strVal, scope) == 1){
												printf("Error: can't minus one a function...\n");
											}else{
												check_arith($lvalue, yylval.strVal);
												$term = newexpr(var_e);
												$term->sym = newtemp();
												if($lvalue->type == tableitem_e){
													struct expr* val = emit_iftableitem($lvalue, currQuad, yylineno);
													emit(assign, val , NULL, $term, currQuad, yylineno);
													emit(sub , val , newexpr_constnum(1), val, currQuad, yylineno);
													emit(tablesetelem , $lvalue , $lvalue->index, val, currQuad, yylineno);
												}
												else{
													emit(assign , $lvalue , NULL, $term, currQuad, yylineno);
													emit(sub , $lvalue , newexpr_constnum(1), $lvalue, currQuad, yylineno);
												}
											}
										}
		|L_PAREN expr R_PARENR			{printf("%d: term-> ( expr )\n", yylineno);
											$term = $expr;
										}
		;

primary:	 lvalue						{
											printf("%d: primary-> lval\n", yylineno);
											$primary = emit_iftableitem($lvalue, currQuad, yylineno);
										}
			|call 						{printf("%d: primary-> call\n", yylineno); }
			|objectdef 					{printf("%d: primary-> objectdef\n", yylineno);}
			|constant 						{printf("%d: primary-> const\n", yylineno);
											$primary = newexpr(constnum_e);
											printf("%d: primary-> $const = %d\n", yylineno, yylval.intVal);
										
											$primary = $constant;
											$primary->sym = NULL;
											printf("%d: primary-> const = %d\n", yylineno, $$->intConst);

										}
			|L_PAREN funcdef R_PARENR 	{printf("%d: primary-> ( functdef )\n", yylineno);
											$primary = newexpr(programfunc_e);
											$primary->sym = $funcdef->sym;
										}
			;

lvalue:	 member 						{printf("%d: lvalue-> member\n", yylineno);
											$lvalue = $member;
										}
		|ID 							{printf("%d: lvalue-> id\n", yylineno);
											printf("scope %d  strval = %s\n", scope, yylval.strVal);

											if(scope == 0){
												if(LookupInScopeVar(scope, yylval.strVal, "var") == 0){
													sym = InsertVar(hashTable, ++key, yylval.strVal, 0, yylineno, 0);
													flagError = 2;
													
												}
											}
											else{
												if(in_func_block_flag > 0){
													if(LookupInScopeVar(0, yylval.strVal, "var") == 0){
														if(Lookup("var", yylval.strVal, scope-1) == 1){
															printf("Error: Cannot access var %s in line %d \n", yylval.strVal, yylineno);
															flagError = 1;
														}
														else{
														if(LookupInScopeVar(scope, yylval.strVal, "var") == 0){
															printf("=In ID before insert tail = %d, localoffset = %d\n", tailOffSet->localvaroffset, functionLocalOffset);
															sym = InsertVar(hashTable, ++key, yylval.strVal, scope, yylineno, 1);
															flagError = 2;
														}
													}
													}
												}
												else{
													if(Lookup("var", yylval.strVal, scope) == 0){
														sym = InsertVar(hashTable, ++key, yylval.strVal, scope, yylineno, 1);
														flagError = 2;
													}
												}
											}
											if(flagError != 1){

												if (flagError == 2) $lvalue = lvalue_expr(sym);
												else {
													$lvalue = newexpr(var_e);
													$lvalue->sym->name = yylval.strVal;
												}
												if($lvalue->sym->name == NULL){
													$lvalue->sym->name = yylval.strVal;
												}

													expression = $lvalue;
													bool_id_flag == 1;

											
												
													
											}
											
											flagError = 0;	
											bool_id_flag = 1;
											bool_op_flag = 0;
										

										}
		|LOCALVAR ID 					{printf("%d: lvalue-> local id\n", yylineno);
										if(LookupInScopeVar(scope, yylval.strVal, "local") == 0){
											sym = InsertVar(hashTable, ++key, yylval.strVal, scope, yylineno, 1);

											$lvalue = newexpr(var_e);
											$lvalue->sym->name = yylval.strVal;

											bool_id_flag = 1;

										}
										$lvalue = lvalue_expr(sym);
										}
		|DOUBLE_COL ID 					{printf("%d: lvalue-> double col id\n", yylineno);
											if(LookupInScopeVar(0, yylval.strVal, "var") == 0){
												printf("Error. ::%s not defined.",yylval.strVal);
												flagError = 1;
											}
											if(flagError != 1){
												$lvalue = lvalue_expr(sym);
												flagError = 0;		
											}
										}
		;
		
member:	 lvalue DOT ID				{
										printf("%d: member-> lvalue.id\n", yylineno);

										$member = member_item($lvalue, $ID, currQuad, yylineno);

									}
		|call DOT ID 						{printf("%d: member-> call.id\n", yylineno);}
		|lvalue {	memlval = yylval.strVal; memFlag = 1;	} L_SBRACKET expr R_SBRACKET {
																							printf("%d: member-> lvalue [ expr ]\n", yylineno);
																							$lvalue = emit_iftableitem($lvalue, currQuad, yylineno);
																							
																							$member = newexpr(tableitem_e);
																							$member->sym = $lvalue->sym;
																							$member->index = $expr;
																						}
		|call L_SBRACKET expr R_SBRACKET 	{printf("%d: member-> call [ expr ]\n", yylineno);}
		;


call:	call L_PAREN	{
							paramCount = 0;
						} elistlist R_PARENR								{printf("%d: call-> call(elist)\n", yylineno);

																		$$ = make_call($1, $elistlist, currQuad, yylineno);

																		if(elists != NULL) paramCount = elists->countParams;
																		$elistlist = NULL;
																		
																	}
		|lvalue	{
					paramCount = 0;
				} callsuffix											{printf("%d: call-> lval callsuffix = %s\n", yylineno, $lvalue->sym->name);
																		$lvalue = emit_iftableitem($lvalue, currQuad, yylineno);
																		if($callsuffix->method){
																			struct expr* t = $lvalue;
																			$lvalue = emit_iftableitem(member_item(t, $callsuffix->name, currQuad, yylineno), currQuad, yylineno);
																			PushElist(t);
																			paramCount = elists->countParams;
																		}
																		$call = make_call($lvalue, elists, currQuad, yylineno);
																	}
		|L_PAREN funcdef R_PARENR L_PAREN	{
												paramCount = 0;
											} elistlist R_PARENR 		{printf("%d: call-> ( funcdef ) ( elist )\n", yylineno);
																		struct expr* func = newexpr(programfunc_e);
																		func->sym = $funcdef-> sym;
																		$call = make_call(func, $elistlist, currQuad, yylineno);
																		$elistlist = NULL;
																		
																	};



callsuffix:		 normcall 				{printf("%d:callsuffix-> normcall = %d\n", yylineno, $normcall->method);
										$callsuffix = $normcall;				
									}
			|methodcall				{printf("%d:callsuffix-> methodcall\n", yylineno);
										$callsuffix = $methodcall;									
									}
			;



normcall:		L_PAREN elistlist R_PARENR				{printf("%d: normcall-> ( elistlist )\n", yylineno);
															$$ = (struct calls*) malloc(sizeof(struct calls));
															$$->elist = $2;
															$$->method = 0;
															$$->name = NULL;

														}
														;

			
methodcall:  DOUBLE_DOT ID L_PAREN elistlist R_PARENR 	{
															printf("%d: methodcall-> ..id ( elist )\n", yylineno);
															$$ = (struct calls*) malloc(sizeof(struct calls));
															$$->elist = $4;
															$$->method = 1;
															$$->name = $ID;
														}
			;

elistlist: 	elist						{printf("%d: elistlist-> elist\n", yylineno);

										$elistlist = elists;
										elist = NULL;
															
										}
			|							{printf("%d: elistlist-> no elistlist\n", yylineno); }
			;


elist:	expr {PushElist($1); bool_flag = 0; bool_op_flag = 0;} elists					{printf("%d: elist-> expr elists\n", yylineno);}
		|expr							{printf("%d: elist-> expr\n", yylineno);
											PushElist($1);
											bool_flag = 0;
											bool_op_flag = 0;
											/*emit(param, $expr, NULL, NULL, currQuad, yylineno);*/
										}
		;

elists:  COMMA expr {printf("%d: elists-> COMMA expr elists after ,expr\n", yylineno);
						PushElist($2);
						bool_flag = 0;
						bool_op_flag = 0;
						/*emit(param, $expr, NULL, NULL, currQuad, yylineno);*/

					} elists			{printf("%d: elists-> COMMA expr elists\n", yylineno);}
		|COMMA expr 					{printf("%d: elists-> COMMA expr \n", yylineno);
											PushElist($2);
											bool_flag = 0;
											bool_op_flag = 0;
											/*emit(param, $expr, NULL, NULL, currQuad, yylineno);*/
										}
		;

objectdef:	 L_SBRACKET R_SBRACKET				{printf("%d: objectdef-> [ ]\n", yylineno);
													struct expr* t = newexpr(newtable_e);
													t->sym = newtemp();
													emit(tablecreate, t, NULL, NULL, currQuad, yylineno);
													$objectdef = t;
												}

			|L_SBRACKET elist R_SBRACKET		{printf("%d: objectdef-> [ elist ]\n", yylineno);
													struct expr* t = newexpr(newtable_e);
													t->sym = newtemp();
													emit(tablecreate, t, NULL, NULL, currQuad, yylineno);
													int k;
													int i = elists->countParams;
													for(k = 0 ; k<i; k++){
														emit(tablesetelem, t, newexpr_constnum(k), elists, currQuad, yylineno);
														PopElist();
													}
													$objectdef = t;
												}
			|L_SBRACKET indexed R_SBRACKET 		{printf("%d: objectdef-> [ indexed ]\n", yylineno);
													struct expr* t = newexpr(newtable_e);
													t->sym = newtemp();
													emit(tablecreate, t, NULL, NULL, currQuad, yylineno);

													while(table_head->next){
														emit(tablesetelem, t, table_head->index, table_head->val, currQuad, yylineno);
														PopTable();
													}
													
													$objectdef = t;
												}
			;


indexed:	indexedelems					{printf("%d: indexed-> indexedelems\n", yylineno); }
			;

indexedelems:	L_BRACKET expr COL expr R_BRACKET {PushTable($2, $4);} indexedelem	{printf("%d: indexedelems-> { expr : expr }\n", yylineno); } 
				|L_BRACKET expr COL expr R_BRACKET 				{printf("%d: indexedelem-> { expr : expr }\n", yylineno);
																	PushTable($2, $4);
																}
				;

indexedelem:	COMMA L_BRACKET expr COL expr R_BRACKET {PushTable($3, $5);} indexedelem	{printf("%d: indexedelem-> { expr : expr }\n", yylineno);}
				|COMMA L_BRACKET expr COL expr R_BRACKET {printf("%d: indexedelem-> { expr : expr }\n", yylineno); PushTable($3, $5);}
				;
				
block:	 L_BRACKET {scope++; NewScope(scope); } stmts {} R_BRACKET	{Hide (scope);  scope--; printf("%d: block-> { stmts }\n", yylineno); $block = $stmts;}
		|L_BRACKET {scope++;} R_BRACKET					{Hide (scope);scope--;printf("%d: block-> { }\n", yylineno);}
		;
	

funcname: ID {	printf("%d: funcname\n", yylineno);
				$funcname = (char*) malloc(sizeof(char)*100); 
             	$funcname = yylval.strVal;
             }
		| { printf("%d: funcname\n", yylineno);
			$funcname= (char*) malloc(sizeof(char)*10); 
			sprintf($funcname, "$f%d", funcnamenum);
			funcnamenum ++;
		}
		;

funcprefix: FUNCTION funcname { printf("%d: funcprefix, $funcname = %s\n", yylineno, $funcname);
								if(LookUpinScopeFunc(scope, $funcname, "func") == 0){
									$funcprefix = (struct expr*)malloc(sizeof(struct expr));
									$funcprefix->sym = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
									$funcprefix->sym = InsertFunc(hashTable, ++key, $funcname, scope, yylineno, 0, funcargs);
									$funcprefix->sym->iaddress = nextquadlabel();
									AddOffset(functionLocalOffset);
									functionLocalOffset= 0;
									AddFuncname ($funcname, scope);
									emit(funcstart, $funcprefix, NULL, NULL, currQuad, yylineno);
								}
								else{
									printf("Error: Function with this name already exists.\n");

								}
								enterscopespace();
								formalArgOffset = 0;
							}
							;

funcargs:  L_PAREN 
			{
				printf("%d: funcargs after (\n", yylineno);
				scope++; 
				NewScope(scope); 
				} idlist R_PARENR 
					{ printf("%d: funcargs\n", yylineno);
						enterscopespace();
						in_func_block_flag++; 
						ResetArgs();
						scope--;
					}
					;

funcblockstart: {push(loophead, loopcounter); loopcounter = 0;};
funcblockend:   {loopcounter = pop(loophead);};

funcbody: funcblockstart block funcblockend {	printf("%d: funcbody\n", yylineno);
					$funcbody = functionLocalOffset;
					exitscopespace();
				}


funcdef:		funcprefix funcargs funcbody {
					printf("%d: funcdef\n", yylineno);
					exitscopespace();
					$funcprefix->sym->totalLocals = $funcbody;
					int offset = func_locals_offset;
					functionLocalOffset = tailOffSet->localvaroffset;
					Rmoffset();
					$funcdef = $funcprefix;
					emit(funcend, $funcprefix, NULL, NULL, currQuad, yylineno);

				}
			;

constant:	 INT 			{printf("%d: const-> int = %d\n", yylineno, $INT);
								$constant = newexpr(constnum_e);
								$constant->intConst = $INT;
							}
		|REAL			{printf("%d: const-> real\n", yylineno);
						$constant = newexpr(constnum_e);
						$constant->numConst = $REAL;
						
						}
		|STRING 		{printf("%d: const-> string = %s\n", yylineno, yylval.strVal);
						$constant = newexpr(conststring_e);
						$constant->strConst = (char*) malloc(sizeof(char)*strlen($STRING));
						strcpy($constant->strConst, $STRING);
						}

		|NIL			{printf("%d: const-> nil\n", yylineno);
						$constant = newexpr(nil_e);
						$constant->strConst = (char*) malloc(sizeof(char)*strlen($NIL));
						strcpy($constant->strConst, $NIL);}

		|TRUE			{printf("%d: const-> true\n", yylineno);
						$constant = newexpr(constbool_e);
						$constant->boolConst = 'T';
						}

		|FALSE			{printf("%d: const-> false\n", yylineno);
						$constant = newexpr(constbool_e);
						$constant->boolConst = 'F';
						}
		;

idlist:	ids 		{printf("%d: idlist-> ids\n", yylineno);}
		|			{printf("%d: idlist-> no idlist\n", yylineno);}
		;
		
ids:	 ID			{AddArguement(yylval.strVal, scope);
						printf("formalArgOffset = %d\n", formalArgOffset);
					}
			idss {printf("%d: ids-> id idss\n", yylineno);}
		|ID			{printf("%d: ids-> id\n", yylineno); AddArguement(yylval.strVal, scope); }

		;
		
idss:	 COMMA ID 		{ AddArguement(yylval.strVal, scope);  }
		idss		{printf("%d: idss-> , id idss\n", yylineno);}
		|COMMA ID			{printf("%d: idss-> , id\n", yylineno); AddArguement(yylval.strVal, scope); }
		;

ifprefix: IF L_PAREN expr R_PARENR { printf("%d: ifprefix-> if ( expr )\n", yylineno);
										bool_flag = 0;
										bool_op_flag = 0;
										struct expr* true_expr = newexpr_constbool('T');
										struct expr* jump_int = newexpr(jump);
										jump_int->intConst = currQuad + 2;

										emit(if_eq, $expr, true_expr, jump_int, currQuad, yylineno);
										$ifprefix = currQuad;
										emit(jump, NULL, NULL, NULL, currQuad, yylineno);
}
;

elseprefix: ELSE {printf("%d: elseprefix-> else\n", yylineno);
					bool_flag = 0;
					bool_op_flag = 0;
					$elseprefix = currQuad;
					emit(jump, NULL, NULL, NULL, currQuad, yylineno);
}
;

ifstmt: ifprefix stmt {printf("%d: ifstmt-> ifprefix stmt\n", yylineno);
						patchlabel($ifprefix, currQuad);
						$ifstmt = $stmt;
						bool_flag = 0;
						bool_op_flag = 0;
						}

		|ifprefix stmt elseprefix stmt{printf("%d: ifstmt-> ifprefix stmt elseprefix stmt\n", yylineno);
										$ifstmt = make_stmt ();
										patchlabel($ifprefix, $elseprefix+1);
										bool_flag = 0;
										bool_op_flag = 0;
										patchlabel($elseprefix, currQuad);
										$ifstmt->breaklist = mergelist($2->breaklist, $4->breaklist);
										$ifstmt->contlist = mergelist($2->contlist, $4->contlist);
		}
		;

whilestart: WHILE {printf("%d: whilestart-> while\n", yylineno);
					$whilestart = currQuad;
}
;

whilecond: L_PAREN expr R_PARENR {printf("%d: whilecond-> ( expr )\n", yylineno);
									bool_flag = 0;
									bool_op_flag = 0;
									struct expr* true_expr = newexpr_constbool('T');
									struct expr* jump_int = newexpr(jump);

									jump_int->intConst = currQuad + 2;
									emit(if_eq, $expr, true_expr, jump_int, currQuad, yylineno);
									$whilecond = currQuad;
									emit(jump, NULL, NULL, NULL, currQuad, yylineno);
}
;

whilestmt: whilestart whilecond loopstmt {printf("%d: whilestmt-> while ( expr ) stmt\n", yylineno);
										struct expr* jump_int = newexpr(jump);
										jump_int->intConst = $whilestart;
										emit(jump, NULL, NULL, jump_int, currQuad, yylineno);
										patchlabel($whilecond, currQuad);
										
										if($loopstmt){
											patchlist($loopstmt->breaklist, currQuad);
											patchlist($loopstmt->contlist, $whilestart);
										}
}
;


N: {$N = currQuad; emit(jump, NULL, NULL, NULL, currQuad, yylineno);};
L: {$L = currQuad;};

forprefix: FOR  L_PAREN elistlist SEMICOL L expr SEMICOL {printf("%d: forprefix-> for( elist ; L expr\n", yylineno);
															bool_flag = 0;
															bool_op_flag = 0;
															$forprefix = (struct enter_test*) malloc(sizeof(struct enter_test));
															$forprefix->test = $L;
															$forprefix->enter = currQuad;
															struct expr* true_expr = newexpr_constbool('T');
															emit(if_eq, $expr, true_expr, NULL, currQuad, yylineno);
}
;
forstmt: forprefix N elistlist R_PARENR N loopstmt N {printf("%d: whilestmt-> forstmt-> for ( N elist; expr; elist ) N stmt N\n", yylineno);
													patchlabel($1->enter, $5+1);
													patchlabel($2, currQuad);
													patchlabel($5, $1->test);
													patchlabel($7, $2+1);

													patchlist($loopstmt->breaklist, currQuad);
													patchlist($loopstmt->contlist, $2+1);

};

forstmtlll:	FOR L_PAREN elistlist SEMICOL expr SEMICOL elistlist R_PARENR stmt	{printf("%d: whilestmt-> forstmt-> for ( elist; expr; elist ) stmt\n", yylineno);}
			;

loopstart: {++loopcounter;};
loopend:	{--loopcounter;};
loopstmt:   loopstart stmt loopend	{$loopstmt = $stmt;
				bool_flag = 0;
				bool_op_flag = 0;
				};

returnstmt:	 RETURN SEMICOL 			{printf("%d: returnstmt-> return;\n", yylineno);
											emit(returnn, NULL, NULL, NULL, currQuad, yylineno);
										}
			|RETURN expr SEMICOL 		{printf("%d: returnstmt-> return expr;\n", yylineno);
			bool_flag = 0;
			bool_op_flag = 0;
											emit(returnn, $expr, NULL, NULL, currQuad, yylineno);
										}
			;
			

%%
void Backpatch (struct true_false* list, unsigned M){
	struct true_false* tmp = list;
	while(tmp != NULL){
		quads[tmp->index].result = (struct expr*) malloc(sizeof(struct expr));
		quads[tmp->index].result->intConst = M;
		tmp = tmp->next;
	}
}


void ReplaceQuads ( a,  b){
	struct quad tmp = quads[a];
	struct quad tmp2 = quads[a+1];


	tmp.label = quads[b].label;
	tmp2.label = quads[b+1].label;
	quads[b].label = quads[a].label;
	quads[b+1].label = quads[a+1].label;

	quads[a] = quads[b];
	quads[b] = tmp;

	quads[a+1] = quads[b+1];
	quads[b+1] = tmp2;

}
void PushTable (struct expr* index, struct expr* val){

	struct table_items* tmp = (struct table_items*) malloc(sizeof(struct table_items));
	tmp->index = index;
	tmp->val = val;

	if(table_head == NULL){
		table_head = tmp;
	}
	else{
		tmp->next = table_head;
		table_head = tmp;
	}
}

void PopTable (){
	struct table_items* tmp = table_head;
	table_head = table_head->next;
}

struct true_false* merge (struct true_false* list1, struct true_false* list2){
	struct true_false* tmp = list1;
	while(tmp->next != NULL){
		tmp = tmp->next;
	}
	tmp->next = list2;

	return list1;
}

void push_head_elist(){
	struct expr* tmp;
	tmp  = (struct expr*)malloc(sizeof(struct expr));
	if(elists == NULL){
		elists = tmp;
	}
	else{
		tmp->next = elists;
		elists = tmp;
	}
}

void push (struct loopcounter_t* stack, int loopcounter){
	struct loopcounter_t* node;
	node = (struct loopcounter_t*)malloc(sizeof(struct loopcounter_t));
	node->count = loopcounter;
	node->next = NULL;

	if(stack == NULL){
		stack = node;
	}
	else{
		node->next = stack;
		stack = node;
	}
}

int pop (struct loopcounter_t* stack){
	struct loopcounter_t* node = stack;
	stack = stack->next;
	int counter = node->count;
	return counter;
}
void PushElist(struct expr* node){
	if(node == NULL) {
		paramCount++;
		return;
	}
	paramCount++;
	node->countParams = paramCount;
	if(elists == NULL){
		elists = (struct expr*)malloc(sizeof(struct expr));
		elists = node;
	}else{
		node->next = elists;
		elists = node;
	}
}

void AddOffset(int localvaroffset){
	offsets* node;
	offsets* tmp;
	node = (offsets*)malloc(sizeof(offsets));
	node->localvaroffset = localvaroffset;
	node->next = NULL;
	if(offSets->localvaroffset == -1){
		offSets = node;
		tailOffSet = node;
	}else{
		tmp = offSets;
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = node;
		tailOffSet = node;
	}

}

void Rmoffset(){
	offsets* tmp;
	offsets* prev;
	prev = offSets;
	tmp = offSets;
	if(offSets->next == NULL){
		offSets->localvaroffset = 0;
		return;
	}
	while(tmp->next != NULL){
		prev = tmp;
		tmp = tmp->next;
	}
	prev->next = NULL;
	tailOffSet = prev;
}

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
	}else{
		printf("Error: Arguement with the name %s already exists.\n", name);
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
	memlval = (char*)malloc(sizeof(char)*100);
	sym = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
	scopesHeadsList= (ScopesHeadsList*)malloc(sizeof(ScopesHeadsList));
	scopesHeadsList -> next = NULL;
	scopesHeadsList->scopeHead =  NULL;
	funcargs = (FuncArgs*)malloc(sizeof(FuncArgs));
	funcargs->arg = NULL;
	funcargs->next = NULL;

	funcnames = (FuncNames*)malloc(sizeof(FuncNames));
	funcnames->name = NULL;
	funcnames->next = NULL;

	offSets = (offsets*)malloc(sizeof(offsets));
	offSets->next = NULL;
	offSets->localvaroffset = -1;

	expression = (struct expr*)malloc(sizeof(struct expr));
	expression->sym = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
	expression->sym->name = (char*)malloc(sizeof(char)*10);

	lval_for_call = (struct expr*)malloc(sizeof(struct expr));
	lval_for_call->sym = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));

	elists = NULL;

	for_method = (struct calls*)malloc(sizeof(struct calls));
	for_method->elist = (struct expr*)malloc(sizeof(struct expr));

	true_list = (struct expr*)malloc(sizeof(struct expr));
	true_list->sym = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));

	false_list = (struct expr*)malloc(sizeof(struct expr));
	false_list->sym = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));

	loophead = (struct loopcounter_t*)malloc(sizeof(struct loopcounter_t));
	push(loophead, 0);

	elists_heads = (struct expr*)malloc(sizeof(struct expr));
	elists_heads = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));

	table_head = (struct table_items*)malloc(sizeof(struct table_items));
	table_head->index = (struct expr*)malloc(sizeof(struct expr));
	table_head->val = (struct expr*)malloc(sizeof(struct expr));

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
	

   	fp = fopen("quads.txt", "w+");

	yyparse(NULL);
	PrintScopes();
	printf("\n");
	printQuads();
	//PrintHash(hashTable);
	writeQuads();
	fclose(fp);
	return 0;
}

//Επίλογος (προαιρετικό)