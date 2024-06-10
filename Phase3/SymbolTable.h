#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define HASH_MULTIPLIER 67458
#define MAX_SIZE 1021

enum scopespace_t{
	programvar,	functionlocal,	formalarg
};


enum iopcode{
	assign, 		add, 			sub,
	mul,			divv,			mod,
	uminus,			and,			or,
	not,			if_eq,			if_noteq,
	if_lesseq,		if_greatereq,	if_less,
	if_greater,		getretval,		funcstart,
	funcend,		tablecreate,	param,
	tablegetelem,	tablesetelem,	call,
	jump,			returnn
};

enum expr_t {
	var_e,	tableitem_e,	
	programfunc_e,	libraryfunc_e,
	arithexpr_e,	 boolexpr_e,	assignexpr_e,	 newtable_e,
	constnum_e,	constbool_e,	conststring_e,
	nil_e
};

enum SymbolType {
	GLOBAL, LOCAL, FORMAL,
	USERFUNC, LIBFUNC
};

typedef struct offsets
{
	int localvaroffset;
	struct offsets* next;
}offsets;

typedef struct Variable {
	const char *name;
	unsigned int scope;
	unsigned int line;
	enum SymbolType type;
} Variable;

typedef struct FuncArgs {
	char *arg;
	enum SymbolType FORMAL;
	struct FuncArgs* next;
}FuncArgs;

typedef struct Function {
	const char *name;
	FuncArgs *head;
	enum SymbolType type;
	unsigned int scope;
	unsigned int line;
}Function;

typedef struct SymbolTableEntry {
	char *key;
	int totalLocals;
	int isActive;
	const char *name;
	unsigned iaddress;
	FuncArgs *head;
	unsigned int scope;
	unsigned int line;
	unsigned int offset;
	enum SymbolType type;
	enum scopespace_t space;
	struct SymbolTableEntry *scopeNext;
	struct SymbolTableEntry *next;
}SymbolTableEntry;

typedef struct FuncNames {
	char *name;
	int scope;
	struct FuncNames* next;
}FuncNames;

typedef struct SymbolTable {
	unsigned int length;
	SymbolTableEntry **table;
}SymbolTable;

typedef struct ScopesHeadsList {
	struct SymbolTableEntry *scopeHead;
	struct ScopesHeadsList* next;
}ScopesHeadsList;

struct true_false {
	unsigned index;
	struct true_false* next;
};

struct expr{
	enum expr_t type;
	SymbolTableEntry* sym;
	struct expr* index; 			// gia tin diaxirisi dinamikon pinakon
	double numConst;				// timi sta8eris ekfrasis
	int intConst;
	char* strConst;					// timi sta8eris ekfrasis
	unsigned char boolConst;		// timi sta8eris ekfrasis
	int countParams;
	struct true_false* true_list;
	struct true_false* false_list;
	struct expr* next;				//gia listes apo expr (gia tis entoles call, table create) 
};

struct calls{
	struct expr* elist;
	unsigned method;
	char* name;
};

typedef struct quad {
	enum iopcode op;
	struct expr* result;
	struct expr* arg1;
	struct expr* arg2;
	unsigned label;
	unsigned line;
}quad;

struct enter_test{
	unsigned enter;
	unsigned test;
	struct enter_test* next;
};

struct loopcounter_t{
	int count;
	struct loopcounter_t* next;
};

struct break_cont {
	int label;
	struct break_cont* next;
};
struct stmt_t {
	struct break_cont* breaklist;
	struct break_cont* contlist;
};

struct table_items{
	struct expr* index;
	struct expr* val;
	struct table_items* next;
	
};

quad* quads = (quad*) 0;
unsigned total=0;
unsigned int currQuad = 0;

#define EXPAND_SIZE 1024
#define CURR_SIZE (total*sizeof(struct quad))
#define NEW_SIZE (EXPAND_SIZE*sizeof(struct quad)+CURR_SIZE)


unsigned int programVarOffset = 0;
unsigned int functionLocalOffset = 0;
unsigned int formalArgOffset = 0;
unsigned int scopeSpaceCounter = 1;
unsigned int tempcounter = 0;
int paramCount=0;
struct expr* elists;
ScopesHeadsList *scopesHeadsList;
FILE *fp;


int legthOfScopesHeadsList = 0;

SymbolTableEntry* InsertVar(SymbolTable *hashTable,int key,char *name, unsigned int scope, unsigned int line, int type);
SymbolTableEntry* InsertFunc(SymbolTable *hashTable,int key,char *name, unsigned int scope, unsigned int line, int type, FuncArgs *funcargs);
int LookupInScopeVar(unsigned int scope, char *name, char *type);
void Hide(unsigned int scope);
void PrintScopes();
void PrintHash (SymbolTable *hashTable);
int LookUpinScopeFunc (unsigned int scope, char *name, char *type);
void printQuads(void);
struct expr* newexpr (enum expr_t t);
struct expr* newexpr_conststring (char* s);
struct expr* emit_iftableitem(struct expr* e, unsigned label, unsigned line);
struct expr* member_item (struct expr* lv, char* name, unsigned label, unsigned line);
struct expr* lvalue_expr (SymbolTableEntry* sym);
enum scopespace_t currscopespace(void);
unsigned currscopeoffset(void);
void incurrscopeoffset(void);
void enterscopespace(void);
void exitscopespace(void);
void expand(void);
void emit(enum iopcode op, struct expr* arg1, struct expr* arg2, struct expr* result, unsigned label, unsigned line);
char* newtempname();
SymbolTableEntry* newtemp();
void resettemp(void);
unsigned nextquadlabel(void);
void patchlabel (unsigned quadNo, unsigned label);
void patchlist (struct break_cont* list, int label);
void printQuads(void);
struct expr* make_call (struct expr* lv, struct expr* reversed_elist, unsigned label, unsigned line);
void check_arith (struct expr* e, const char* context);
char* iopcode_to_string (enum iopcode op);
struct stmt_t* make_stmt ();
void newlist(struct break_cont* i);
void PopElist();
void writeQuads(void);
struct break_cont* mergelist (struct break_cont* l1, struct break_cont* l2);

//----------------------PHASE 3 FUNCS-----------------------------------

struct expr* newexpr (enum expr_t t){
	struct expr* e = (struct expr*) malloc (sizeof(struct expr));
	e->sym = (struct SymbolTableEntry*)malloc(sizeof(struct SymbolTableEntry));
	e->false_list = (struct true_false*)malloc(sizeof(struct true_false));
	e->true_list = (struct true_false*)malloc(sizeof(struct true_false));
	e->boolConst = NULL;
	//memset(e, 0, sizeof(struct expr));
	e->type = t;
	return e;
}

struct expr* make_call (struct expr* lv, struct expr* reversed_elist, unsigned label, unsigned line){
	printf("In make_call\n");
	int i;
	struct expr* result;
	int count;


	if(reversed_elist == NULL) count = 0;
	else count = reversed_elist->countParams;

	struct expr* func = emit_iftableitem(lv, currQuad, line);
	printf("jjjjjjjjjjj \n");

	printf("jjjjjjjjjjj \n\n");
	for(i=0;i<count;i++){
		printf("in make call while i = %d to %d\n", i, count);
		emit(param , reversed_elist , NULL, NULL, currQuad, line);
		reversed_elist = reversed_elist-> next;
		PopElist();
	}
	/*while(reversed_elist){
		printf("in make call while %s\n");
		emit(param , reversed_elist , NULL, NULL, currQuad, line);
		reversed_elist = reversed_elist-> next;
	}*/

	printf("after make call while\n");
	emit(call, func, NULL , NULL, currQuad, line);
	result = newexpr (var_e);
	result->sym = newtemp();
	emit(getretval , NULL, NULL, result, currQuad, line);

	return result;
}

struct expr* newexpr_conststring (char* s){
	struct expr* e = newexpr(conststring_e);
	e->sym = NULL;
	e->strConst = strdup(s);
	printf("e->strConst = %s\n",e->strConst);
	return e;
}

struct expr* newexpr_constnum (double n){
	struct expr* e = newexpr(constnum_e);
	e->sym = NULL;
	e->numConst = n;
	printf("e->numConst = %f\n",e->numConst);
	return e;
}

struct expr* newexpr_constbool (unsigned char b){
	struct expr* e = newexpr(constbool_e);
	e->sym = NULL;
	e->boolConst = b;
	printf("e->boolConst = %c\n",e->boolConst);
	return e;
}

struct stmt_t* make_stmt (){
	struct stmt_t* t = (struct stmt_t*) malloc (sizeof(struct stmt_t));
	t->breaklist = (struct break_cont*) malloc (sizeof(struct break_cont));
	t->contlist = (struct break_cont*) malloc (sizeof(struct break_cont));
	t->breaklist->label = -1;
	t->contlist->label = -1;
	t->breaklist->next = NULL;
	t->contlist->next = NULL;
	return t;
}

void newlist(struct break_cont* i){
	i->label = currQuad;
	i->next = NULL;
}

struct break_cont* mergelist (struct break_cont* l1, struct break_cont* l2){
	printf("In mergelist\n");
	if(l1->label == -1){
		printf("in 1st if\n");
		return l2;
	}
	else if (l2->label == -1){
		printf("in 2nd if\n");
		return l1;
	}
	else{
		printf("in 3rd if\n");
		struct break_cont* tmp = l1;
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = l2;
		return l1;
	}
	printf("2\n");
}

struct expr* emit_iftableitem(struct expr* e, unsigned label, unsigned line){
	if(e->type != tableitem_e){
		printf("Type is not tableitem_e\n");
		return e;
	}
		struct expr* result = newexpr(var_e);
		result->sym = newtemp();
		emit(tablegetelem, e, e->index, result, label, line);
		if(result->sym == NULL) printf("result = null'n");
		printf("jjjjjj result = %s\n", result->sym->name);
		return result;
}

struct expr* member_item (struct expr* lv, char* name, unsigned label, unsigned line){
	printf("in member_item\n");
	char* type = (char*)malloc(sizeof(char)*20);
	if(lv->type == var_e){
		strcpy(type, "var_e");
	}else if(lv->type == programfunc_e){
		strcpy(type, "programfunc_e");
	}else if(lv->type == libraryfunc_e){
		strcpy(type, "libraryfunc_e");
	}
	lv = emit_iftableitem(lv, label, line);// Emit code if r value use of table item
	struct expr* ti = newexpr(tableitem_e);// Make a new expression

	ti ->sym = lv->sym;
	ti->index = newexpr_conststring(name);// Const string index
	return ti;
}

struct expr* lvalue_expr (SymbolTableEntry* sym){
	assert(sym);
	struct expr* e = (struct expr*)malloc(sizeof(struct expr));
	e->false_list = (struct true_false*)malloc(sizeof(struct true_false));
	e->true_list = (struct true_false*)malloc(sizeof(struct true_false));
	e->boolConst = NULL;
	memset(e, 0, sizeof(struct expr));
	
	char* type = (char*)malloc(sizeof(char)*20);
	e->next = (struct expr*) 0;
	e->sym = sym;
	if(sym->type == LOCAL){
				strcpy(type, "Local variable");
			}else if(sym->type == GLOBAL){
				strcpy(type, "Global variable");
			}else if(sym->type == FORMAL){
				strcpy(type, "Formal variable");
			}else if(sym->type == USERFUNC){
				strcpy(type, "User function");
			}else if(sym->type == LIBFUNC) {
				strcpy(type, "Library function");
			}

	if(sym->type == LOCAL || sym->type == GLOBAL || sym->type == FORMAL){
		e->type = var_e;
	}else if(sym->type == USERFUNC){
		e->type = programfunc_e;
	}else{
		e->type = libraryfunc_e;
	}	
	

	return e;
}

void PopElist(){
	printf("IN POPELIST\n");
	struct expr* tmp = elists;
	elists = elists->next;
	if(elists == NULL) paramCount = 0;
	else paramCount = elists->countParams;

}

void patchlist (struct break_cont* list, int label){
	struct break_cont*tmp = list;
	printf("----------In patchlist = %d\n", list->label);
	if(tmp->label == -1) return;
	while(tmp){
		quads[tmp->label].result = (struct expr*) malloc(sizeof(struct expr));
		quads[tmp->label].result->intConst = label;
		tmp = tmp->next;
	}
}

void check_arith (struct expr* e, const char* context) {
if (e->type == constbool_e || e->type == conststring_e || e->type == nil_e || e->type == newtable_e || e->type == programfunc_e || e->type == libraryfunc_e || e->type == boolexpr_e )
		printf("Illegal expr used in %s!", context);
}

enum scopespace_t currscopespace(void){
	if(scopeSpaceCounter == 1)
		return programvar;
	else if (scopeSpaceCounter % 2 == 0)
		return formalarg;
	else
		return functionlocal;
}

unsigned currscopeoffset(void){
	switch (currscopespace())
	{
	case programvar:
		return programVarOffset;
	case functionlocal:
		return functionLocalOffset;
	case formalarg:
		return formalArgOffset;	
	default:
		assert(0);
	}
}

void incurrscopeoffset(void){
	switch (currscopespace())
	{
	case programvar:
		++programVarOffset;
		break;
	case functionlocal:
		++functionLocalOffset;
		break;
	case formalarg:
		++formalArgOffset;
		break;
	default:
		assert(0);
	}
}

void enterscopespace(void){
	++scopeSpaceCounter;
}

void exitscopespace(void){
	assert(scopeSpaceCounter>1);
	--scopeSpaceCounter;
}

void expand(void){
	assert(total == currQuad);
	quad* p = (quad*) malloc(NEW_SIZE);
	if(quads){
		memcpy(p, quads, CURR_SIZE);
		free(quads);
	}
	quads = p;
	total +=EXPAND_SIZE;
}

void emit(enum iopcode op, struct expr* arg1, struct expr* arg2, struct expr* result, unsigned label, unsigned line){
	if(currQuad == total){
		printf("Before expand\n");
		expand();
	}
	char* sop = (char*)malloc(sizeof(char)*15);
	printf("In emit\n");
	quad* p = quads+currQuad++;
	p->op = op;
	p->arg1 = arg1;
	p->arg2 = arg2;
	p->result = result;
	p->label = label;
	p->line = line;
	

		sop = iopcode_to_string(op);


		if((strcmp(sop,"funcend") == 0) || (strcmp(sop,"funcstart") == 0) || (strcmp(sop,"call") == 0)){
			printf("-op = %s \n", sop);
			printf("%d: %s %s [line %d]\n", p->label, sop, p->arg1->sym->name, p->line);
		}
		else if((strcmp(sop,"param") == 0)){
			printf("%d: \n", p->label);

			if(p->arg1->sym) 				printf("%s %s ", sop, p->arg1->sym->name);	
			else if(p->arg1->intConst) 		printf("%s %d ", sop, p->arg1->intConst);
			else if(p->arg1->numConst) 		printf("%s %f ", sop, p->arg1->numConst);
			else if(p->arg1->boolConst) 	printf("%s %c ", sop, p->arg1->boolConst);
			else 							printf("%s %s ", sop, p->arg1->strConst);

			printf("[line %d] \n", p->line);

			//printf("%d: %s %s [line %d]\n", quads[i].label, op, quads[i].arg1->sym->name, quads[i].line);
		}
		else if(strcmp(sop,"assign") == 0){
			printf("-op = %s \n", sop);
			/*printf("op = assign\n");
			printf("arg1 = %s\n", p->arg1->sym->name);
			printf("result = %s\n", p->result->sym->name);*/
			if(p->arg1->sym) 				printf("%d: %s %s %s [line %d]\n", p->label, sop, p->arg1->sym->name, p->result->sym->name, p->line);
			else if(p->arg1->intConst) 	printf("%d: %s %d %s [line %d]\n", p->label, sop, p->arg1->intConst,  p->result->sym->name, p->line);
			else if(p->arg1->numConst) 	printf("%d: %s %f %s [line %d]\n", p->label, sop, p->arg1->numConst,  p->result->sym->name, p->line);
			else if(p->arg1->boolConst) 	printf("%d: %s %c %s [line %d]\n", p->label, sop, p->arg1->boolConst, p->result->sym->name, p->line);
			else 								printf("%d: %s %s %s [line %d]\n", p->label, sop, p->arg1->strConst, p->result->sym->name, p->line);
		}else if(strcmp(sop,"tablegetelem") == 0 ){
			printf("-op = %s \n", sop);
			/*printf("op = tablegetelem\n");
			printf("label = %d\n",quads[i].label);
			printf("op = %s\n", op);
			printf("arg1 = %s\n", quads[i].arg1->sym->name);
			printf("arg2 = %s\n", quads[i].arg2->strConst);
			printf("result = %s\n", quads[i].result->sym->name);
			printf("line = %d\n", quads[i].line);*/
			
			printf("%d: %s %s \"%s\" %s [line %d]\n", p->label, sop, p->arg1->sym->name, p->arg2->strConst, p->result->sym->name, p->line);
		}else if( strcmp(sop,"tablesetelem") == 0){
			printf("-op = %s \n", sop);
			//printf("op = tablesetelem\n");
			printf("%d: %s %s \"%s\" ", p->label, sop, p->arg1->sym->name, p->arg2->strConst);

			if 		(p->result->sym) 				printf("%s [line %d]\n", p->result->sym->name, p->line);
			else if	(p->result->numConst != 0)  	printf("%f [line %d]\n", p->result->numConst, p->line);
			else if (p->result->strConst != NULL) 	printf("%s [line %d]\n", p->result->strConst, p->line);
			else 									printf("%d [line %d]\n", p->result->intConst, p->line);
			
		}
		else if( strcmp(sop,"add") == 0 || strcmp(sop,"sub") == 0 || strcmp(sop,"mul") == 0 || strcmp(sop,"divv") == 0 || strcmp(sop,"mod") == 0){
			printf("-op = %s \n", sop);
			if (p->arg1->sym) 						printf("%d: %s %s ", p->label, sop, p->arg1->sym->name);
			else if(p->arg1->numConst != 0)  		printf("%d: %s %f ", p->label, sop, p->arg1->numConst);
			else if (p->arg1->strConst != NULL) 	printf("%d: %s %s ", p->label, sop, p->arg1->strConst);
			else 									printf("%d: %s %d ", p->label, sop, p->arg1->intConst);

			if (p->arg2->sym) 						printf("%s %s [line %d]\n", p->arg2->sym->name, p->result->sym->name, p->line);
			else if(p->arg2->numConst != 0)  		printf("%f %s [line %d]\n", p->arg2->numConst, p->result->sym->name, p->line);
			else if (p->arg2->strConst != NULL) 	printf("%s %s [line %d]\n", p->arg2->strConst, p->result->sym->name, p->line);
			else 									printf("%d %s [line %d]\n", p->arg2->intConst, p->result->sym->name, p->line);

		}
		else if(strcmp(sop,"uminus") == 0 ){
			printf("-op = %s \n", sop);
			printf("%d: %s %s %s [line %d]\n", p->label, sop, p->arg1->sym->name, p->result->sym->name, p->line);
		}
		else if(strcmp(sop,"tablecreate") == 0){
			printf("-op = %s \n", sop);
			printf("%d: %s %s [line %d]\n", p->label, sop, p->arg1->sym->name, p->line);
		}
		else if(strcmp(sop,"jump") == 0){

			if(p->result) printf("%d: %s %d [line %d]\n", p->label, sop, p->result->intConst, p->line);
			else printf("%d: %s _ [line %d]\n", p->label, sop, p->line);
		}
		else if(strcmp(sop,"return") == 0){
			if(p->arg1 == NULL){
				printf("%d: %s [line %d]\n", p->label, sop, p->line);
			}
			else{
				if (p->arg1->sym) 						printf("name %d: %s %s ", p->label, sop, p->arg1->sym->name);
				else if(p->arg1->numConst != 0)  		printf("double %d: %s %f ", p->label, sop, p->arg1->numConst);
				else if (p->arg1->strConst != NULL) 	printf("string %d: %s %s ", p->label, sop, p->arg1->strConst);
				else if (p->arg1->boolConst != NULL) 	printf("Bool %d: %s %c ", p->label, sop, p->arg1->boolConst);
				else 									printf("int %d: %s %d ", p->label, sop, p->arg1->intConst);

				printf("[line %d]\n", p->line);
			}
		}
		else if(strcmp(sop,"getretval") == 0){
			printf("%d: %s %s [line %d]\n", p->label, sop, p->result->sym->name, p->line);
		}
		else{
			printf("op = ta ipolipa\n");
			printf("%d: %s %s %s %s [line %d]\n", p->label, sop, p->arg1, p->arg2, p->result, p->line);
		}
printQuads();

}

char* newtempname(){
	char* new_var;
	new_var= (char*) malloc(sizeof(char)*5);
	sprintf(new_var, "_t%d", tempcounter);
	tempcounter++;
	return new_var;
}

/*expr* newexpr(int type){
	expr* exp;
	exp = (expr*) malloc (sizeof(expr)); 
	exp->sym = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
	switch (type)
	{
	case 1:
		exp->type = var_e;
		break;
	case 2:
		exp->type = tableitem_e;
		break;
	case 3:
		exp->type = programfunc_e;
		break;
	case 4:
		exp->type = libraryfunc_e;
		break;
	case 5:
		exp->type = arithexpr_e;
		break;
	case 6:
		exp->type = boolexpr_e;
		break;
	case 7:
		exp->type = assignexpr_e;
		break;
	case 8:
		exp->type = newtalbe_e;
		break;
	case 9:
		exp->type = constnum_e;
		break;
	case 10:
		exp->type = constbool_e;
		break;
	case 11:
		exp->type = conststring_e;
		break;
	default:
		exp->type = nil_e;
		break;
	}
	exp->next = NULL;
	return exp;
}*/

SymbolTableEntry* newtemp(){
	SymbolTableEntry* node;
	char*name;
	node = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
	name = newtempname();
	node->name = name;
	return node;
	

}

void resettemp(void){
	tempcounter = 0;
}

unsigned nextquadlabel(void){
	return currQuad;
}

void patchlabel (unsigned quadNo, unsigned label){
	assert(quadNo<currQuad);
	quads[quadNo].result = (struct expr*) malloc(sizeof(struct expr));
	quads[quadNo].result->intConst = label;
}

char* iopcode_to_string (enum iopcode op){

	char* sop = (char*)malloc(sizeof(char)*15);
	if(op == assign){
			strcpy(sop, "assign");
		}else if(op == add){
			strcpy(sop, "add");
		}else if(op == sub){
			strcpy(sop, "sub");
		}else if(op == mul){
			strcpy(sop, "mul");
		}else if(op == divv) {
			strcpy(sop, "divv");
		}else if(op == mod) {
			strcpy(sop, "mod");
		}else if(op == uminus) {
			strcpy(sop, "uminus");
		}else if(op == and) {
			strcpy(sop, "and");
		}else if(op == or) {
			strcpy(sop, "or");
		}else if(op == not) {
			strcpy(sop, "not");
		}else if(op == if_eq) {
			strcpy(sop, "if_eq");
		}else if(op == if_noteq) {
			strcpy(sop, "if_noteq");
		}else if(op == if_lesseq) {
			strcpy(sop, "if_lesseq");
		}else if(op == if_greatereq) {
			strcpy(sop, "if_greatereq");
		}else if(op == if_less) {
			strcpy(sop, "if_less");
		}else if(op == if_greater) {
			strcpy(sop, "if_greater");
		}else if(op == getretval) {
			strcpy(sop, "getretval");
		}else if(op == funcstart) {
			strcpy(sop, "funcstart");
		}else if(op == tablecreate) {
			strcpy(sop, "tablecreate");
		}else if(op == funcend) {
			strcpy(sop, "funcend");
		}else if(op == tablegetelem) {
			strcpy(sop, "tablegetelem");
		}else if(op == tablesetelem) {
			strcpy(sop, "tablesetelem");
		}else if(op == param) {
			strcpy(sop, "param");
		}else if(op == call) {
			strcpy(sop, "call");
		}else if(op == jump){
			strcpy(sop, "jump");
		}
		else if(op == returnn){
			strcpy(sop, "return");
		}
		return sop;
}

void printQuads(void){
	int i =0;
	char* op = (char*)malloc(sizeof(char)*15);
		
	for(i=0;i<currQuad;i++){
		op = iopcode_to_string(quads[i].op);


		if((strcmp(op,"funcend") == 0) || (strcmp(op,"funcstart") == 0) || (strcmp(op,"call") == 0)){
			printf("%d: %s %s [line %d]\n", quads[i].label, op, quads[i].arg1->sym->name, quads[i].line);
		}
		else if((strcmp(op,"param") == 0)){
			printf("%d: ", quads[i].label);

			if(quads[i].arg1->sym) 					printf("sym %s %s ", op, quads[i].arg1->sym->name);	
			else if(quads[i].arg1->intConst) 		printf("int %s %d ", op, quads[i].arg1->intConst);
			else if(quads[i].arg1->numConst != 0) 		printf("num %s %f ", op, quads[i].arg1->numConst);
			else if(quads[i].arg1->boolConst == 'T' && quads[i].arg1->boolConst == 'F' ) 		printf("bool %s %c ", op, quads[i].arg1->boolConst);
			else 									printf("str %s %s ", op, quads[i].arg1->strConst);

			printf("[line %d] \n", quads[i].line);

			//printf("%d: %s %s [line %d]\n", quads[i].label, op, quads[i].arg1->sym->name, quads[i].line);
		}
		else if(strcmp(op,"assign") == 0){
			/*printf("op = assign\n");
			printf("arg1 = %s\n", quads[i].arg1->sym->name);
			printf("result = %s\n", quads[i].result->sym->name);
			*/
			if (quads[i].arg1->sym) 					printf("%d: %s %s ", quads[i].label, op, quads[i].arg1->sym->name);
			else if(quads[i].arg1->boolConst)  			printf("%d: %s %c ", quads[i].label, op, quads[i].arg1->boolConst);
			else if(quads[i].arg1->numConst != 0)  		printf("%d: %s %f ", quads[i].label, op, quads[i].arg1->numConst);
			else if (quads[i].arg1->strConst != NULL) 	printf("%d: %s %s ", quads[i].label, op, quads[i].arg1->strConst);
			else 										printf("%d: %s %d ", quads[i].label, op, quads[i].arg1->intConst);

			if (quads[i].result->sym) 						printf("sym %s [line %d]\n", quads[i].result->sym->name, quads[i].line);
			else if(quads[i].result->numConst != 0)  		printf("num %f [line %d]\n", quads[i].result->numConst, quads[i].line);
			else if (quads[i].result->strConst != NULL) 	printf("str %s [line %d]\n", quads[i].result->strConst, quads[i].line);
			else 											printf("int %d [line %d]\n", quads[i].result->intConst, quads[i].line);

		}
		else if( strcmp(op,"tablesetelem") == 0 || strcmp(op,"tablegetelem") == 0 ){

			//printf("op = tablesetelem\n");
			if (quads[i].arg2->sym) 						printf("%d: %s %s %s ", quads[i].label, op, quads[i].arg1->sym->name, quads[i].arg2->sym->name);
			else if (quads[i].arg2->numConst != 0) 			printf("%d: %s %s %f ", quads[i].label, op, quads[i].arg1->sym->name, quads[i].arg2->numConst);
			else if (quads[i].arg2->strConst != NULL) 		printf("%d: %s %s %s ", quads[i].label, op, quads[i].arg1->sym->name, quads[i].arg2->strConst);
			else 											printf("%d: %s %s %d ", quads[i].label, op, quads[i].arg1->sym->name, quads[i].arg2->intConst);
			
			if 		(quads[i].result->sym) 					printf("%s [line %d]\n", quads[i].result->sym->name, quads[i].line);
			else if	(quads[i].result->numConst != 0)  		printf("%f [line %d]\n", quads[i].result->numConst,  quads[i].line);
			else if (quads[i].result->strConst != NULL) 	printf("%s [line %d]\n", quads[i].result->strConst,  quads[i].line);
			else 											printf("%d [line %d]\n", quads[i].result->intConst,  quads[i].line);
			
		}
		else if( strcmp(op,"add") == 0 || strcmp(op,"sub") == 0 || strcmp(op,"mul") == 0 || strcmp(op,"divv") == 0 || strcmp(op,"mod") == 0){


			if (quads[i].arg1->sym) 					printf("%d: %s %s ", quads[i].label, op, quads[i].arg1->sym->name);
			else if(quads[i].arg1->numConst != 0)  		printf("%d: %s %f ", quads[i].label, op, quads[i].arg1->numConst);
			else if (quads[i].arg1->strConst != NULL) 	printf("%d: %s %s ", quads[i].label, op, quads[i].arg1->strConst);
			else 										printf("%d: %s %d ", quads[i].label, op, quads[i].arg1->intConst);

			if (quads[i].arg2->sym) 					printf("%s %s [line %d]\n", quads[i].arg2->sym->name, quads[i].result->sym->name, quads[i].line);
			else if(quads[i].arg2->numConst != 0)  		printf("%f %s [line %d]\n", quads[i].arg2->numConst, quads[i].result->sym->name, quads[i].line);
			else if (quads[i].arg2->strConst != NULL) 	printf("%s %s [line %d]\n", quads[i].arg2->strConst, quads[i].result->sym->name, quads[i].line);
			else 										printf("%d %s [line %d]\n", quads[i].arg2->intConst, quads[i].result->sym->name, quads[i].line);

		}
		else if(strcmp(op,"uminus") == 0 ){
			printf("%d: %s %s %s [line %d]\n", quads[i].label, op, quads[i].arg1->sym->name,  quads[i].result->sym->name, quads[i].line);
		}
		else if(strcmp(op,"tablecreate") == 0){
			printf("%d: %s %s [line %d]\n", quads[i].label, op, quads[i].arg1->sym->name, quads[i].line);
		}
		else if(strcmp(op,"jump") == 0){

			if(quads[i].result) printf("%d: %s %d [line %d]\n", quads[i].label, op, quads[i].result->intConst, quads[i].line);
			else printf("%d: %s _ [line %d]\n", quads[i].label, op, quads[i].line);
		}
		else if(strcmp(op,"if_lesseq")==0 || strcmp(op,"if_greatereq") == 0 || strcmp(op,"if_less") ==0 || strcmp(op,"if_greater") ==0 || strcmp(op,"if_eq") ==0 || strcmp(op,"if_noteq") ==0){
	
			if (quads[i].arg1->sym) 					printf("%d: %s %s ", quads[i].label, op, quads[i].arg1->sym->name);
			else if(quads[i].arg1->boolConst)  			printf("%d: %s %c ", quads[i].label, op, quads[i].arg1->boolConst);
			else if(quads[i].arg1->numConst != 0)  		printf("%d: %s %f ", quads[i].label, op, quads[i].arg1->numConst);
			else 										printf("%d: %s %d ", quads[i].label, op, quads[i].arg1->intConst);

			if (quads[i].arg2->sym) 					printf("%s ", quads[i].arg2->sym->name);
			else if(quads[i].arg2->numConst != 0)  		printf("%f ", quads[i].arg2->numConst);
			else if (quads[i].arg2->boolConst)			printf("%c ", quads[i].arg2->boolConst);
			else 										printf("%d ", quads[i].arg2->intConst);

			if (quads[i].result) printf("%d [line %d]\n", quads[i].result->intConst, quads[i].line);
			else printf("_ [line %d]\n", quads[i].line);
		}
		else if(strcmp(op,"return") == 0){
			if(quads[i].arg1 == NULL){
				printf("%d: %s [line %d]\n", quads[i].label, op, quads[i].line);
			}
			else{
				if (quads[i].arg1->sym) 						printf("%d: %s %s ", quads[i].label, op, quads[i].arg1->sym->name);
				else if(quads[i].arg1->strConst != NULL)  		printf("%d: %s %s ", quads[i].label, op, quads[i].arg1->strConst);
				else if (quads[i].arg1->numConst != 0) 	printf("%d: %s %f ", quads[i].label, op, quads[i].arg1->numConst);
				else if (quads[i].arg1->boolConst != NULL) 	printf("%d: %s %c ", quads[i].label, op, quads[i].arg1->boolConst);
				else 									printf("%d: %s %d ", quads[i].label, op, quads[i].arg1->intConst);

				printf("[line %d]\n", quads[i].line);
			}
		}
		else if(strcmp(op,"getretval") == 0){
			printf("%d: %s %s [line %d]\n", quads[i].label, op, quads[i].result->sym->name, quads[i].line);
		}
		else{
			printf("%d: %s %s %s %s [line %d]\n", quads[i].label, op, quads[i].arg1, quads[i].arg2, quads[i].result, quads[i].line);
		}
	}	

}


void writeQuads(void){
	int i =0;
	char* op = (char*)malloc(sizeof(char)*15);
	char* forfile = (char*)malloc(sizeof(char)*1500);
		
	for(i=0;i<currQuad;i++){
		op = iopcode_to_string(quads[i].op);


		if((strcmp(op,"funcend") == 0) || (strcmp(op,"funcstart") == 0) || (strcmp(op,"call") == 0)){
			fprintf(fp, "%d: %s %s [line %d]\n", quads[i].label, op, quads[i].arg1->sym->name, quads[i].line);


		}
		else if((strcmp(op,"param") == 0)){
			fprintf(fp, "%d: ", quads[i].label);
			

			if(quads[i].arg1->sym) 																fprintf(fp, "sym %s %s ", op, quads[i].arg1->sym->name);	
			else if(quads[i].arg1->intConst) 													fprintf(fp, "int %s %d ", op, quads[i].arg1->intConst);
			else if(quads[i].arg1->numConst != 0) 												fprintf(fp, "num %s %f ", op, quads[i].arg1->numConst);
			else if(quads[i].arg1->boolConst == 'T' && quads[i].arg1->boolConst == 'F' ) 		fprintf(fp, "bool %s %c ", op, quads[i].arg1->boolConst);
			else 																				fprintf(fp, "str %s %s ", op, quads[i].arg1->strConst);
			

			fprintf(fp, "[line %d] \n", quads[i].line);


			//printf("%d: %s %s [line %d]\n", quads[i].label, op, quads[i].arg1->sym->name, quads[i].line);
		}
		else if(strcmp(op,"assign") == 0){
			/*printf("op = assign\n");
			printf("arg1 = %s\n", quads[i].arg1->sym->name);
			printf("result = %s\n", quads[i].result->sym->name);
			*/
			if (quads[i].arg1->sym) 					fprintf(fp, "%d: %s %s ", quads[i].label, op, quads[i].arg1->sym->name);
			else if(quads[i].arg1->boolConst)  			fprintf(fp, "%d: %s %c ", quads[i].label, op, quads[i].arg1->boolConst);
			else if(quads[i].arg1->numConst != 0)  		fprintf(fp, "%d: %s %f ", quads[i].label, op, quads[i].arg1->numConst);
			else if (quads[i].arg1->strConst != NULL) 	fprintf(fp, "%d: %s %s ", quads[i].label, op, quads[i].arg1->strConst);
			else 										fprintf(fp, "%d: %s %d ", quads[i].label, op, quads[i].arg1->intConst);


			if (quads[i].result->sym) 						fprintf(fp, "%s [line %d]\n", quads[i].result->sym->name, quads[i].line);
			else if(quads[i].result->numConst != 0)  		fprintf(fp, "%f [line %d]\n", quads[i].result->numConst, quads[i].line);
			else if (quads[i].result->strConst != NULL) 	fprintf(fp, "%s [line %d]\n", quads[i].result->strConst, quads[i].line);
			else 											fprintf(fp, "%d [line %d]\n", quads[i].result->intConst, quads[i].line);


		}else if( strcmp(op,"tablesetelem") == 0 || strcmp(op,"tablegetelem") == 0 ){

			//printf("op = tablesetelem\n");
			if (quads[i].arg2->sym) 						fprintf(fp, "%d: %s %s %s ", quads[i].label, op, quads[i].arg1->sym->name, quads[i].arg2->sym->name);
			else if (quads[i].arg2->numConst != 0) 			fprintf(fp, "%d: %s %s %f ", quads[i].label, op, quads[i].arg1->sym->name, quads[i].arg2->numConst);
			else if (quads[i].arg2->boolConst) 				fprintf(fp, "%d: %s %s %f ", quads[i].label, op, quads[i].arg1->sym->name, quads[i].arg2->boolConst);
			else if (quads[i].arg2->strConst != NULL) 		fprintf(fp, "%d: %s %s %s ", quads[i].label, op, quads[i].arg1->sym->name, quads[i].arg2->strConst);
			else 											fprintf(fp, "%d: %s %s %d ", quads[i].label, op, quads[i].arg1->sym->name, quads[i].arg2->intConst);

			if 		(quads[i].result->sym) 					fprintf(fp, "%s [line %d]\n", quads[i].result->sym->name, quads[i].line);
			else if	(quads[i].result->numConst != 0)  		fprintf(fp, "%f [line %d]\n", quads[i].result->numConst,  quads[i].line);
			else if (quads[i].result->strConst != NULL) 	fprintf(fp, "%s [line %d]\n", quads[i].result->strConst,  quads[i].line);
			else 											fprintf(fp, "%d [line %d]\n", quads[i].result->intConst,  quads[i].line);
			
		}
		else if( strcmp(op,"add") == 0 || strcmp(op,"sub") == 0 || strcmp(op,"mul") == 0 || strcmp(op,"divv") == 0 || strcmp(op,"mod") == 0){


			if (quads[i].arg1->sym) 					fprintf(fp, "%d: %s %s ", quads[i].label, op, quads[i].arg1->sym->name);
			else if(quads[i].arg1->numConst != 0)  		fprintf(fp, "%d: %s %f ", quads[i].label, op, quads[i].arg1->numConst);
			else if (quads[i].arg1->strConst != NULL) 	fprintf(fp, "%d: %s %s ", quads[i].label, op, quads[i].arg1->strConst);
			else 										fprintf(fp, "%d: %s %d ", quads[i].label, op, quads[i].arg1->intConst);

			

			if (quads[i].arg2->sym) 					fprintf(fp, "%s %s [line %d]\n", quads[i].arg2->sym->name, quads[i].result->sym->name, quads[i].line);
			else if(quads[i].arg2->numConst != 0)  		fprintf(fp, "%f %s [line %d]\n", quads[i].arg2->numConst, quads[i].result->sym->name, quads[i].line);
			else if (quads[i].arg2->strConst != NULL) 	fprintf(fp, "%s %s [line %d]\n", quads[i].arg2->strConst, quads[i].result->sym->name, quads[i].line);
			else 										fprintf(fp, "%d %s [line %d]\n", quads[i].arg2->intConst, quads[i].result->sym->name, quads[i].line);

			

		}
		else if(strcmp(op,"uminus") == 0 ){
			fprintf(fp, "%d: %s %s %s [line %d]\n", quads[i].label, op, quads[i].arg1->sym->name,  quads[i].result->sym->name, quads[i].line);
			
		}
		else if(strcmp(op,"tablecreate") == 0){
			fprintf(fp, "%d: %s %s [line %d]\n", quads[i].label, op, quads[i].arg1->sym->name, quads[i].line);
			
		}
		else if(strcmp(op,"jump") == 0){

			if(quads[i].result) fprintf(fp, "%d: %s %d [line %d]\n", quads[i].label, op, quads[i].result->intConst, quads[i].line);
			else fprintf(fp, "%d: %s _ [line %d]\n", quads[i].label, op, quads[i].line);
			
		}
		else if(strcmp(op,"if_lesseq")==0 || strcmp(op,"if_greatereq") == 0 || strcmp(op,"if_less") ==0 || strcmp(op,"if_greater") ==0 || strcmp(op,"if_eq") ==0 || strcmp(op,"if_noteq") ==0){

			if (quads[i].arg1->sym) 					fprintf(fp, "%d: %s %s ", quads[i].label, op, quads[i].arg1->sym->name);
			else if(quads[i].arg1->boolConst)  			fprintf(fp, "%d: %s %c ", quads[i].label, op, quads[i].arg1->boolConst);
			else if(quads[i].arg1->numConst != 0)  		fprintf(fp, "%d: %s %f ", quads[i].label, op, quads[i].arg1->numConst);
			else 										fprintf(fp, "%d: %s %d ", quads[i].label, op, quads[i].arg1->intConst);

			

			if (quads[i].arg2->sym) 					fprintf(fp, "%s ", quads[i].arg2->sym->name);
			else if(quads[i].arg2->numConst != 0)  		fprintf(fp, "%f ", quads[i].arg2->numConst);
			else if (quads[i].arg2->boolConst)			fprintf(fp, "%c ", quads[i].arg2->boolConst);
			else 										fprintf(fp, "%d ", quads[i].arg2->intConst);

			

			if (quads[i].result) fprintf(fp, "%d [line %d]\n", quads[i].result->intConst, quads[i].line);
			else fprintf(fp, "_ [line %d]\n", quads[i].line);
			
		}
		else if(strcmp(op,"return") == 0){
			if(quads[i].arg1 == NULL){
				fprintf(fp, "%d: %s [line %d]\n", quads[i].label, op, quads[i].line);
				
			}
			else{
				if (quads[i].arg1->sym) 						fprintf(fp, "%d: %s %s ", quads[i].label, op, quads[i].arg1->sym->name);
				else if(quads[i].arg1->strConst != NULL)  		fprintf(fp, "%d: %s %s ", quads[i].label, op, quads[i].arg1->strConst);
				else if (quads[i].arg1->numConst != 0) 			fprintf(fp, "%d: %s %f ", quads[i].label, op, quads[i].arg1->numConst);
				else if (quads[i].arg1->boolConst != NULL) 		fprintf(fp, "%d: %s %c ", quads[i].label, op, quads[i].arg1->boolConst);
				else 											fprintf(fp, "%d: %s %d ", quads[i].label, op, quads[i].arg1->intConst);
				
				fprintf(fp, "[line %d]\n", quads[i].line);
				
			}
		}
		else if(strcmp(op,"getretval") == 0){
			fprintf(fp, "%d: %s %s [line %d]\n", quads[i].label, op, quads[i].result->sym->name, quads[i].line);
			
		}
		else{
			fprintf(fp, "%d: %s %s %s %s [line %d]\n", quads[i].label, op, quads[i].arg1, quads[i].arg2, quads[i].result, quads[i].line);
			
		}
	}	

}

//---------------------PHASE 2 FUNCS----------------------------------
static unsigned int SymbolTable_hash(int key) {

    size_t ui;
    unsigned int uiHash = 0U;

	uiHash = uiHash * HASH_MULTIPLIER + key;
    return uiHash;

}

SymbolTable *SymTable_new(void){
	int i;
    SymbolTable *v;
    v=(struct SymbolTable*)malloc(sizeof(struct SymbolTable));
	v->table = (SymbolTableEntry**) malloc (sizeof(SymbolTableEntry*)*MAX_SIZE);
	v -> length = MAX_SIZE;
	for(i=0; i<MAX_SIZE; i++){
		v -> table[i] = NULL;
	}
    return v;
}

void NewScope (int scope ){

	ScopesHeadsList *tmp;
	ScopesHeadsList *nxt = (ScopesHeadsList*)malloc(sizeof(ScopesHeadsList));
	int i=0;

	nxt->next=NULL;

	nxt->scopeHead=NULL;
	if(legthOfScopesHeadsList < scope){
		legthOfScopesHeadsList++;
		scopesHeadsList = realloc(scopesHeadsList, sizeof(ScopesHeadsList)*(legthOfScopesHeadsList+1));

		tmp = scopesHeadsList;
		for(i=1;i<scope;i++){
			tmp = tmp -> next; 
		}
		if(tmp->next == NULL){
			tmp->next = nxt;
			
		}
	}

}

SymbolTableEntry* InsertVar(SymbolTable *hashTable,int key,char *name, unsigned int scope, unsigned int line, int type){

	SymbolTableEntry *node, *tmp2;
	ScopesHeadsList *tmp;
	ScopesHeadsList *nxt = (ScopesHeadsList*)malloc(sizeof(ScopesHeadsList));
	int i;
	unsigned int hash = SymbolTable_hash(key)%MAX_SIZE;
	node = (SymbolTableEntry*) malloc(sizeof(SymbolTableEntry));
	node->offset = currscopeoffset();
	incurrscopeoffset();
	node->isActive = 1;
	node->name = name;
	node->scope = scope;
	node->line = line;
	switch (type)
	{
	case 1:
		node->type = LOCAL;
		break;
	case 2:
		node->type = FORMAL;
		break;
	default:
		node->type = GLOBAL;
		break;
	}
	node->next = NULL;
	node->scopeNext = NULL;

	nxt->next=NULL;
	nxt->scopeHead=node;


	if(legthOfScopesHeadsList < scope){
		legthOfScopesHeadsList++;
		scopesHeadsList = realloc(scopesHeadsList, sizeof(ScopesHeadsList)*legthOfScopesHeadsList);
	}
	node->next = hashTable->table[hash];
	hashTable->table[hash]=node;
	if(scopesHeadsList->scopeHead == NULL){
		scopesHeadsList -> scopeHead = node;
		scopesHeadsList -> next = NULL;
		
		return node;
	}
	tmp = scopesHeadsList;
	if(scope == 0){
		tmp2 = tmp->scopeHead;
		while(tmp2->scopeNext!=NULL){
			tmp2=tmp2->scopeNext;
		}
		tmp2->scopeNext = node;
		tmp2 = tmp2->scopeNext;
		tmp2->scopeNext = NULL;
		return node;
	}
	for(i=0;i<scope;i++){
		tmp = tmp -> next; 
	}
	if(tmp->next==NULL){
		tmp->next = nxt; 
	}
	else if(tmp->next != NULL && tmp->next->scopeHead ==NULL){
		tmp->next->scopeHead = node;
		tmp->next->scopeHead->next = NULL;
	}
	else{
		tmp2= tmp->next->scopeHead;
		while(tmp2->scopeNext!=NULL){
			tmp2=tmp2->scopeNext;
		}
		tmp2->scopeNext = node;
		tmp2 = tmp2->scopeNext;
		tmp2->scopeNext = NULL;
	}

	return node;
	
}


SymbolTableEntry* InsertFunc(SymbolTable *hashTable,int key,char *name, unsigned int scope, unsigned int line, int type, FuncArgs *funcargs){

	
	SymbolTableEntry *node, *tmp2;
	ScopesHeadsList *tmp;

	ScopesHeadsList *nxt = (ScopesHeadsList*)malloc(sizeof(ScopesHeadsList));

	int i;
	unsigned int hash = SymbolTable_hash(key)%MAX_SIZE;
	
	node = (SymbolTableEntry*) malloc(sizeof(SymbolTableEntry));
	node->space = currscopespace();
	node->offset = currscopeoffset();
	node->isActive = 1;
	node->name = name;
	node->scope = scope;
	node->line = line;
	node->iaddress = nextquadlabel();
	
	if(type == 1){
		node->type = LIBFUNC;
	}else {
		node->type = USERFUNC;
	}

	node->head = funcargs;
	node->next = NULL;
	node->scopeNext = NULL;

	nxt->next=NULL;
	nxt->scopeHead=node;



	if(legthOfScopesHeadsList < scope){
		legthOfScopesHeadsList++;
		scopesHeadsList = realloc(scopesHeadsList, sizeof(ScopesHeadsList)*legthOfScopesHeadsList);
	}

	node->next = hashTable->table[hash];
	hashTable->table[hash]=node;
	if(scopesHeadsList->scopeHead == NULL){
		scopesHeadsList -> scopeHead = node;
		scopesHeadsList -> next = NULL;
		
		return node;
	}

	tmp = scopesHeadsList;

	if(scope == 0){
		
		tmp2 = tmp->scopeHead;
		while(tmp2->scopeNext!=NULL){
			tmp2=tmp2->scopeNext;
		}
		tmp2->scopeNext = node;
		tmp2 = tmp2->scopeNext;
		tmp2->scopeNext = NULL;
		return node;
	}
	for(i=0;i<scope;i++){
		tmp = tmp -> next; 
	}
	if(tmp->next==NULL){
		tmp->next = nxt; 
	}
	else if(tmp->next != NULL && tmp->next->scopeHead ==NULL){
		tmp->next->scopeHead = node;
		tmp->next->scopeHead->next = NULL;
	}
	else{
		tmp2= tmp->next->scopeHead;
		while(tmp2->scopeNext!=NULL){
			tmp2=tmp2->scopeNext;
		}
		tmp2->scopeNext = node;
		tmp2 = tmp2->scopeNext;
		tmp2->scopeNext = NULL;
	}

	return node;
}

void InsertFuncArgs (char* argname, char* fname);
/* 1 if exists
else 0*/
int LookupInScopeVar(unsigned int scope, char *name, char *type){

	int i=0;
	ScopesHeadsList *tmp = scopesHeadsList;

	SymbolTableEntry *tmp2;

	if (scope != 0 && strcmp(type, "local")==0 ){
		if(strcmp(name, "print") == 0 || strcmp(name, "input") == 0 || strcmp(name, "objectmemberkeys") == 0 || strcmp(name, "objecttotalmembers") == 0 || strcmp(name, "objectcopy") == 0 || strcmp(name, "totalarguments") == 0 || strcmp(name, "argument") == 0 || strcmp(name, "typeof") == 0 ||strcmp(name, "strtonum") == 0 ||strcmp(name, "sqrt") == 0 ||strcmp(name, "cos") == 0 ||strcmp(name, "sin") == 0){
			printf("Error: %s trying to shadow libfunc\n", name);
			return 2;
		}
		
	}

	if(scope != 0){
		while(i<=scope){
			if(tmp->next == NULL) {
				return 0;
			}
			tmp = tmp->next;
			i++;
		}
	}

if(tmp->scopeHead == NULL){
		return 0;
	}
	tmp2 = tmp->scopeHead;

	while(tmp2!=NULL){

		if(strcmp(tmp2->name, name) == 0 && tmp2->isActive == 1){

			return 1;
		}
		tmp2=tmp2->scopeNext;
	}

	return 0;
}

/* 1 if exists
else 0*/
int Lookup(char *ttype, char *name, unsigned int scope){
	int i;


	for(i=scope;i>=0;i--){
		if(LookupInScopeVar(i, name, ttype)==1){
			printf("Collision with existing variable!\n");
			return 1;
		}
	}
	return 0;
}


/* 1 if exists
else 0*/
int LookUpinScopeFunc (unsigned int scope, char *name, char *type){

	int i=0;

	
	ScopesHeadsList *tmp = scopesHeadsList;
	SymbolTableEntry *tmp2;
	if(strcmp(name, "print") == 0 || strcmp(name, "input") == 0 || strcmp(name, "objectmemberkeys") == 0 || strcmp(name, "objecttotalmembers") == 0 || strcmp(name, "objectcopy") == 0 || strcmp(name, "totalarguments") == 0 || strcmp(name, "argument") == 0 || strcmp(name, "typeof") == 0 ||strcmp(name, "strtonum") == 0 ||strcmp(name, "sqrt") == 0 ||strcmp(name, "cos") == 0 ||strcmp(name, "sin") == 0){
		printf("Error: %s trying to shadow libfunc\n", name);
		return 2;
	}
	if(scope != 0){
		while(i<=scope){
			if(tmp->next == NULL) {
				return 0;
			}
			tmp = tmp->next;
			i++;
		}
	}

	if(tmp->scopeHead == NULL){
		return 0;
	}
	tmp2 = tmp->scopeHead;
	while(tmp2!=NULL){
		if(strcmp(tmp2->name, name) == 0 && tmp2->isActive == 1){
			return 1;
		}
		tmp2=tmp2->scopeNext;
	}
	return 0;
}

void Hide(unsigned int scope){


	int i;
	ScopesHeadsList *tmp = scopesHeadsList;
	SymbolTableEntry *tmp2;
	for(i=0;i<scope;i++){
		tmp = tmp->next;
	}
	
	if(tmp == NULL){
		return;
	}

	tmp2 = tmp->scopeHead;

	if(tmp2 == NULL){ 
		 return;
	}
	while(tmp2!=NULL){
		if(tmp2->type==LOCAL || tmp2->type==FORMAL){
			tmp2->isActive = 0;
			
		}
		tmp2=tmp2->scopeNext;
	}

}


void PrintScopes(){
	ScopesHeadsList *tmp = scopesHeadsList;
	SymbolTableEntry *tmp2;
	char *type;
	type = (char *) malloc(sizeof(char *)*30);


	while (tmp->next != NULL){
		if(tmp->scopeHead != NULL){
			printf("---------------  Scope: #%d ---------------\n", tmp->scopeHead->scope);
		tmp2 = tmp->scopeHead;
		while (tmp2->scopeNext != NULL)
		{
			if(tmp2->type == LOCAL){
				strcpy(type, "Local variable");
			}else if(tmp2->type == GLOBAL){
				strcpy(type, "Global variable");
			}else if(tmp2->type == FORMAL){
				strcpy(type, "Formal variable");
			}else if(tmp2->type == USERFUNC){
				strcpy(type, "User function");
			}else if(tmp2->type == LIBFUNC) {
				strcpy(type, "Library function");
			}
			/*
			switch(tmp2->type){
				case 0: strcpy(type, "Global variable");
				case 1: strcpy(type, "Local variable");
				case 2: strcpy(type, "Formal variable");
				case 3: strcpy(type, "User function");
				default: strcpy(type, "Library function");
			}
			*/
			printf("  \"%s\"   [%s]  (line %d) (scope %d) isActive: %d\n",tmp2->name, type,  tmp2->line, tmp2->scope, tmp2->isActive);
			tmp2=tmp2->scopeNext;
		}
		if(tmp2->type == LOCAL){
				strcpy(type, "Local variable");
			}else if(tmp2->type == GLOBAL){
				strcpy(type, "Global variable");
			}else if(tmp2->type == FORMAL){
				strcpy(type, "Formal variable");
			}else if(tmp2->type == USERFUNC){
				strcpy(type, "User function");
			}else if(tmp2->type == LIBFUNC) {
				strcpy(type, "Library function");
			}
		printf("  \"%s\"   [%s]  (line %d) (scope %d) isActive: %d\n",tmp2->name, type,  tmp2->line, tmp2->scope, tmp2->isActive);
		
		}
		tmp= tmp->next;
	}
	


	if(tmp->scopeHead != NULL){
			printf("---------------  Scope: #%d ---------------\n", tmp->scopeHead->scope);
		tmp2 = tmp->scopeHead;
		if(tmp2->type == LOCAL){
				strcpy(type, "Local variable");
			}else if(tmp2->type == GLOBAL){
				strcpy(type, "Global variable");
			}else if(tmp2->type == FORMAL){
				strcpy(type, "Formal variable");
			}else if(tmp2->type == USERFUNC){
				strcpy(type, "User function");
			}else if(tmp2->type == LIBFUNC) {
				strcpy(type, "Library function");
			}
		while (tmp2->scopeNext != NULL)
		{
			if(tmp2->type == LOCAL){
				strcpy(type, "Local variable");
			}else if(tmp2->type == GLOBAL){
				strcpy(type, "Global variable");
			}else if(tmp2->type == FORMAL){
				strcpy(type, "Formal variable");
			}else if(tmp2->type == USERFUNC){
				strcpy(type, "User function");
			}else if(tmp2->type == LIBFUNC) {
				strcpy(type, "Library function");
			}
			printf("  \"%s\"   [%s]  (line %d) (scope %d) isActive: %d\n",tmp2->name, type, tmp2->line, tmp2->scope, tmp2->isActive);
			tmp2=tmp2->scopeNext;
		}
		if(tmp2->type == LOCAL){
				strcpy(type, "Local variable");
			}else if(tmp2->type == GLOBAL){
				strcpy(type, "Global variable");
			}else if(tmp2->type == FORMAL){
				strcpy(type, "Formal variable");
			}else if(tmp2->type == USERFUNC){
				strcpy(type, "User function");
			}else if(tmp2->type == LIBFUNC) {
				strcpy(type, "Library function");
			}
		printf("  \"%s\"   [%s]  (line %d) (scope %d) isActive: %d\n",tmp2->name, type, tmp2->line, tmp2->scope, tmp2->isActive);

		}
}

void PrintHash (SymbolTable *hashTable){

	printf("-------------------------\n");
	int i =0;
	for(i; i < MAX_SIZE; i++){
		SymbolTableEntry *tmp = hashTable->table[i];
		while (tmp)
		{
			if(tmp)
			printf("%d: %s",tmp->line, tmp->name );
			else if (!tmp)
			printf("%d: %s",tmp->line, tmp->name );

			tmp = tmp->next;
		}
		printf("\n");
	}
	printf("-------------------------\n");
}



