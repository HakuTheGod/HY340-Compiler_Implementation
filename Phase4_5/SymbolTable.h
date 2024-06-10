#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#define HASH_MULTIPLIER 67458
#define MAX_SIZE 1021




enum scopespace_t{
	programvar,	functionlocal,	formalarg
};

enum vmopcode {
	assign_v			=0,
	add_v				=1,
	sub_v				=2,
	mul_v				=3,
	div_v				=4,
	mod_v				=5,
	uminus_v			=6,
	and_v				=7,
	or_v				=8,
	not_v				=9,
	jeq_v				=10,
	jne_v				=11,
	jle_v				=12,
	jge_v				=13,
	jlt_v				=14,
	jgt_v				=15,
	call_v				=16,
	pusharg_v			=17,
	funcenter_v			=18,
	funcexit_v			=19,
	newtable_v			=20,
	tablegetelem_v		=21,
	tablesetelem_v		=22,
	nop_v				=23,
	jump_v				=24
};

enum vmarg_t{
	label_a		=0,
	global_a	=1,
	formal_a	=2,
	local_a		=3,
	number_a	=4,
	string_a	=5,
	bool_a		=6,
	nil_a		=7,
	userfunc_a	=8,
	libfunc_a	=9,
	retval_a	=10
};

enum avm_memcell_t {
	number_m	=0,
	string_m	=1,
	bool_m		=2,
	table_m		=3,
	userfunc_m	=4,
	libfunc_m	=5,
	nil_m		=6,
	undef_m		=7
};


enum iopcode{
	add, 			sub,			mul,
	divv,			mod,			tablecreate,
	tablegetelem,	tablesetelem,	assign,
	nop,			jump,			if_eq,
	if_noteq,		if_greater,		if_greatereq,
	if_less,		if_lesseq,		not,
	or,				param,			call,
	getretval,		funcstart,		returnn,
	funcend,		and,			uminus
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
	char *name;
	unsigned iaddress;
	unsigned taddress;
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
	unsigned taddress;
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

struct vmarg {
	enum vmarg_t type;
	unsigned val;

};

struct instruction {
	enum vmopcode		opcode;
	struct vmarg*		result;
	struct vmarg*		arg1;
	struct vmarg*		arg2;
	unsigned	srcLine;
};

struct userfunc {
	unsigned	address;
	unsigned	localSize;
	char*		id;
	struct userfunc* next;
};

struct avm_table;

struct avm_memcell {
	enum avm_memcell_t type;
	unsigned			localSize;
	union{
		double			numval;
		char*			strVal;
		unsigned		boolVal;
		struct avm_table*		tableVal;
		unsigned		funcVal;
		char*			libfuncVal;
	} data;
};

struct func_start_end {
	unsigned label;
	struct func_start_end* next;
};


extern void libfunc_print ();
extern void libfunc_typeof ();
extern void libfunc_totalarguments();
extern void libfunc_argument();
extern void libfunc_sqrt ();
extern void libfunc_sin ();
extern void libfunc_cos ();

typedef void (*library_func_t)(void);

library_func_t avm_getlibraryfunc (char*);

library_func_t libfuncs[] = {
	libfunc_print,
	libfunc_typeof,
	libfunc_totalarguments,
	libfunc_argument,
	libfunc_sqrt,
	libfunc_sin,
	libfunc_cos
};




double*				numConsts = (double*) 0;
unsigned			totalNumConsts = 0;
unsigned 			max_const_num = 0;

char**				stringConsts = (char**) 0;
unsigned			totalStringConsts = 0;
unsigned 			max_const_string = 0;

char**				namedLibFuncs = (char**) 0;
unsigned			totalNamedLibfuncs = 0;
unsigned 			max_libs = 0;

struct userfunc*		userFuncs = (struct userfunc*) 0;
struct func_start_end*	userFuncs_stack;
unsigned				totalUserFuncs = 0;
unsigned 				max_user_funcs = 0;

struct avm_memcell* avm_tablegetelem(
    struct avm_table* table,
    struct avm_memcell* index
);

void avm_tablesetelem (
    struct avm_table* table,
    struct avm_memcell* index,
    struct avm_memcell* content
);

//Phase 4-5 our stuff================================

struct func_stack{
	struct SymbolTableEntry* sym;

	struct incomplete_jump* returnList;

	struct func_stack* next;
};




struct func_stack* funcstack;


struct instruction* instructions = (struct instruction*) 0;
unsigned int currInstruction = 0;
unsigned inst_total = 0;
int func_start_end_flag = 0;		//0 = start
									//1 = end

char* func_call_nest;

int call_flag = 0;

unsigned totalActuals = 0;

unsigned string_ptr = 0;
unsigned num_ptr = 0;
unsigned user_ptr = 0;
unsigned lib_ptr = 0;

int flag_totalargs =1;
int flag_call = 0;






//phase 4 end========================================

unsigned globalmem;



quad* quads = (quad*) 0;
unsigned total=0;
unsigned num_of_lib_funcs = 0;
unsigned int currQuad = 0;

#define EXPAND_SIZE 1024
#define CURR_SIZE (total*sizeof(struct quad))
#define NEW_SIZE (EXPAND_SIZE*sizeof(struct quad)+CURR_SIZE)

#define CURR_SIZE_INST (inst_total*sizeof(struct instruction))
#define NEW_SIZE_INST (EXPAND_SIZE*sizeof(struct instruction)+CURR_SIZE_INST)

/*#define CURR_SIZE_NUM (total*sizeof(double))
#define NEW_SIZE_NUM (EXPAND_SIZE*sizeof(double)+CURR_SIZE)
#define CURR_SIZE_STRING (total*sizeof(char*))
#define NEW_SIZE_STRING (EXPAND_SIZE*sizeof(char*)+CURR_SIZE)
#define CURR_SIZE_LIB (total*sizeof(char*))
#define NEW_SIZE_LIB (EXPAND_SIZE*sizeof(char*)+CURR_SIZE)
#define CURR_SIZE_USER (total*sizeof(struct userfunc))
#define NEW_SIZE_USER (EXPAND_SIZE*sizeof(struct userfunc)+CURR_SIZE)*/

#define AVM_STACKSIZE 4096
#define AVM_WIPEOUT(m) memset(&(m), 0, sizeof(m))
#define AVM_STACKENV_SIZE 4
struct avm_memcell ax, bx, cx;
struct avm_memcell retval;
unsigned top, topsp;

// Reverse translation for constants:
// getting constant value from index.


double consts_getnumber (unsigned index);
char* consts_getstring(unsigned index);
char* libfuncs_getused (unsigned index);

struct avm_memcell stack[AVM_STACKSIZE];



#define AVM_TABLE_HASHSIZE 211

struct avm_table_bucket {
	struct avm_memcell			key;
	struct avm_memcell			value;
	struct avm_table_bucket*	next;
};

struct avm_table {
	unsigned			refCounter;
	struct avm_table_bucket*	strIndexed[AVM_TABLE_HASHSIZE];
	struct avm_table_bucket*	numIndexed[AVM_TABLE_HASHSIZE];
	unsigned			total;
};


struct incomplete_jump{
	unsigned					instrNo;
	unsigned					iaddress;
	struct incomplete_jump* 	next;
};


struct incomplete_jump* 	ij_head = (struct incomplete_jump*) 0;
unsigned					ij_total = 0;

extern void	generate_ADD (quad*);
extern void	generate_SUB (quad*);
extern void	generate_MUL (quad*);
extern void	generate_DIV (quad*);
extern void	generate_MOD (quad*);
extern void	generate_NEWTABLE (quad*);
extern void	generate_TABLEGETELEM (quad*);
extern void	generate_TABLESETELEM (quad*);
extern void	generate_ASSIGN (quad*);
extern void	generate_NOP (quad*);
extern void	generate_JUMP (quad*);
extern void	generate_IF_EQ (quad*);
extern void	generate_IF_NOTEQ (quad*);
extern void	generate_GREATER (quad*);
extern void	generate_GREATEREQ (quad*);
extern void	generate_LESS (quad*);
extern void	generate_LESSEQ (quad*);
extern void	generate_NOT (quad*);
extern void	generate_OR (quad*);
extern void	generate_PARAM (quad*);
extern void	generate_CALL (quad*);
extern void	generate_GETRETVAL (quad*);
extern void	generate_FUNCSTART (quad*);
extern void	generate_RETURN (quad*);
extern void	generate_FUNCEND (quad*);
extern void generate_relational (enum vmopcode op, quad* quad);

typedef void (*generator_func_t) (quad*); 

generator_func_t generators[] = {
	generate_ADD,
	generate_SUB,
	generate_MUL,
	generate_DIV,
	generate_MOD,
	generate_NEWTABLE,
	generate_TABLEGETELEM,
	generate_TABLESETELEM,
	generate_ASSIGN,
	generate_NOP,
	generate_JUMP,
	generate_IF_EQ,
	generate_IF_NOTEQ,
	generate_GREATER,
	generate_GREATEREQ,
	generate_LESS,
	generate_LESSEQ,
	generate_NOT,
	generate_OR,
	generate_PARAM,
	generate_CALL,
	generate_GETRETVAL,
	generate_FUNCSTART,
	generate_RETURN,
	generate_FUNCEND
};

unsigned 		executionFinished = 0;
unsigned		pc = 0;
unsigned		currLine = 0;
unsigned		codeSize = 0;
struct instruction*	code = (struct instruction*) 0;
#define AVM_ENDING_PC	code_counter
int code_counter = 0;

#define execute_jge execute_comparison
#define execute_jgt execute_comparison
#define execute_jle execute_comparison
#define execute_jlt execute_comparison

#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic

#define AVM_MAX_INSTRUCTIONS (unsigned) nop_v

extern void execute_assign 		 (struct instruction*);
extern void execute_add 		 (struct instruction*);
extern void execute_sub 		 (struct instruction*);
extern void execute_mul 		 (struct instruction*);
extern void execute_div 		 (struct instruction*);
extern void execute_mod 		 (struct instruction*);
extern void execute_uminus 		 (struct instruction*);
extern void execute_and 		 (struct instruction*);
extern void execute_or 			 (struct instruction*);
extern void execute_not 		 (struct instruction*);
extern void execute_jeq 		 (struct instruction*);
extern void execute_jne 		 (struct instruction*);
extern void execute_jle 		 (struct instruction*);
extern void execute_jge 		 (struct instruction*);
extern void execute_jlt 		 (struct instruction*);
extern void execute_jgt 		 (struct instruction*);
extern void execute_call 		 (struct instruction*);
extern void execute_pusharg 	 (struct instruction*);
extern void execute_funcenter 	 (struct instruction*);
extern void execute_funcexit 	 (struct instruction*);
extern void execute_newtable 	 (struct instruction*);
extern void execute_tablegetelem (struct instruction*);
extern void execute_tablesetelem (struct instruction*);
extern void execute_nop 		 (struct instruction*);



typedef void (*execute_func_t) (struct instruction*);
execute_func_t executeFuncs[] = {
	execute_assign,
	execute_add,
	execute_sub,
	execute_mul,
	execute_div,
	execute_mod,
	execute_uminus,
	execute_and,
	execute_or,
	execute_not,
	execute_jeq,
	execute_jne,
	execute_jle,
	execute_jge,
	execute_jlt,
	execute_jgt, 
	execute_call,
	execute_pusharg,
	execute_funcenter,
	execute_funcexit,
	execute_newtable,
	execute_tablegetelem,
	execute_tablesetelem,
	execute_nop
};

typedef char* (*tostring_func_t) (struct avm_memcell*);

extern char* number_tostring 	(struct avm_memcell*);
extern char* string_tostring	(struct avm_memcell*);
extern char* bool_tostring 		(struct avm_memcell*);
extern char* table_tostring 	(struct avm_memcell*);
extern char* userfunc_tostring 	(struct avm_memcell*);
extern char* libfunc_tostring 	(struct avm_memcell*);
extern char* nil_tostring 		(struct avm_memcell*);
extern char* undef_tostring 	(struct avm_memcell*);

tostring_func_t tostringFuncs [] = {
	number_tostring,
	string_tostring,
	bool_tostring,
	table_tostring,
	userfunc_tostring,
	libfunc_tostring,
	nil_tostring,
	undef_tostring 	
};

unsigned char jge_impl (double x, double y);
unsigned char jgt_impl (double x, double y);
unsigned char jle_impl (double x, double y);
unsigned char jlt_impl (double x, double y);



// Dispatcher just for comparison functions.
typedef unsigned char (*comparison_func_t) (double x, double y);
comparison_func_t comparisonFuncs[] = {
    jle_impl,
    jge_impl,
    jlt_impl,
    jgt_impl
};



double add_impl (double x, double y);
double sub_impl (double x, double y);
double mul_impl (double x, double y);
double div_impl (double x, double y);
double mod_impl (double x, double y);

// Dispatcher just for arithmetic functions.
typedef double (*arithmetic_func_t) (double x, double y);
arithmetic_func_t arithmeticFuncs[] = {
    add_impl,
    sub_impl,
    mul_impl,
    div_impl,
    mod_impl
};

typedef unsigned char (*tobool_func_t) (struct avm_memcell*);

unsigned char number_tobool 	 (struct avm_memcell* m) { return m->data.numval !=0;}
unsigned char string_tobool 	 (struct avm_memcell* m) { return m->data.strVal[0] != 0;}
unsigned char bool_tobool  		 (struct avm_memcell* m) { return m->data.boolVal;}
unsigned char table_tobool  	 (struct avm_memcell* m) { return 1;}
unsigned char userfunc_tobool  	 (struct avm_memcell* m) { return 1;}
unsigned char libfunc_tobool  	 (struct avm_memcell* m) { return 1;}
unsigned char nil_tobool  		 (struct avm_memcell* m) { return 0;}
unsigned char undef_tobool  	 (struct avm_memcell* m) { assert(0); return 0;}

tobool_func_t toboolFuncs[] = {
	number_tobool 	,
	string_tobool 	,
	bool_tobool  	,	
	table_tobool  	,
	userfunc_tobool , 	
	libfunc_tobool  ,	
	nil_tobool  	,	
	undef_tobool  	
};

typedef void (*memclear_func_t) (struct avm_memcell*);

char* typeStrings[] = {
	"number",
	"string",
	"bool",
	"table",
	"userdef",
	"libfunc",
	"nil",
	"undef"
};

void memclear_string (struct avm_memcell* m);
void memclear_table (struct avm_memcell* m);


memclear_func_t memclearFuncs [] = {
	0,					//number
	memclear_string,	//string
	0,					//bool
	memclear_table,		//table
	0,					//userfunc
	0,					//libfunc
	0,					//nil
	0					//undef
};



extern void avm_assign (struct avm_memcell* lv, struct avm_memcell* rv);
void execute_call (struct instruction* instr);


unsigned int programVarOffset = 0;
unsigned int functionLocalOffset = 0;
unsigned int formalArgOffset = 0;
unsigned int scopeSpaceCounter = 1;
unsigned int tempcounter = 0;
int paramCount=0;
struct expr* elists;
ScopesHeadsList *scopesHeadsList;
FILE *fp;
FILE *fp2;
FILE *fpb;


#define AVM_NUMACTUALS_OFFSET 	+4
#define AVM_SAVEDPC_OFFSET		+3
#define AVM_SAVEDTOP_OFFSET		+2
#define AVM_SAVEDTOPSP_OFFSET	+1

/*
unsigned consts_newstring (char* s);
unsigned consts_newnumber (double n);
unsigned libfuncs_newused (char* s);
unsigned userfuncs_newfunc (SymbolTableEntry* sym);

*/

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
static void avm_initstack ();
void avm_tableincrefcounter (struct avm_table* t);
struct break_cont* mergelist (struct break_cont* l1, struct break_cont* l2);
void avm_tablebucketsinit (struct avm_table_bucket** p);
struct avm_table* avm_tablenew ();
void avm_tablebucketsdestroy (struct avm_table_bucket** p);
void avm_tabledestroy (struct avm_table* t);
void make_operand (struct expr* e, struct vmarg* arg);
void make_numberoperand (struct vmarg* arg, double val);
void make_booloperand (struct vmarg* arg, unsigned val);
void make_retvaloperand (struct vmarg* arg);
void add_incomplete_jump (unsigned instrNo, unsigned iaddress);
void patch_incomplete_jumps();
void generate();
unsigned nextinstructionlabel();
void inst_emit(struct instruction* t);
void avm_memcellclear (struct avm_memcell* m);
void generate_inst();
void push_funcstack(SymbolTableEntry* node);
struct func_stack* pop_funcstack();
void append(struct incomplete_jump* ret, int i);
char* vmopcode_to_string (enum vmopcode op);
void expand_consts_num(void);
void expand_consts_string(void);
void expand_consts_libs(void);
void expand_consts_user(void);
void print_instructions ();
int LookUpinScopeFormal (unsigned int scope, char* name);
void execute_circle ();
struct avm_memcell* avm_translate_operand (struct vmarg* arg, struct avm_memcell* reg);
void avm_dec_top ();
void avm_push_envvalue (unsigned val);
extern void avm_callsaveenviroment ();
extern char* avm_tostring (struct avm_memcell*); // Caller frees.
extern void avm_calllibfunc (char* funcName);
unsigned avm_get_envalue (unsigned i);
void pre_exec ();
void print_stack();
void read_binary ();
void expand_code();
void print_code();
unsigned avm_totalactuals();
struct avm_memcell* avm_getactual (unsigned i);
void avm_initialize (void);
library_func_t avm_getlibraryfunc(char* id);
unsigned userfuncs_searchfunc(SymbolTableEntry* sym);


//-----------------------------PHASE 5 FUNCS----------------------------------

// TODO (maybe never)

struct avm_memcell* avm_tablegetelem(struct avm_table* table,struct avm_memcell* index){
	return index;
}


void avm_tablesetelem (struct avm_table* table,struct avm_memcell* index,struct avm_memcell* content){

}



// Useless
void execute_uminus 		(struct instruction* instr){}
void execute_and 		(struct instruction* instr){}
void execute_or 			(struct instruction* instr){}
void execute_jne 		(struct instruction* instr){}
void execute_nop 		 (struct instruction* instr){}
void execute_not 		 (struct instruction* instr){}



char* number_tostring (struct avm_memcell* id){

	char* c = (char*)malloc(sizeof(char)* 50);
	sprintf(c, "%.3f", id->data.numval);
	return c;
}

extern char* string_tostring (struct avm_memcell* id){
	return id->data.strVal;
}

extern char* bool_tostring 		(struct avm_memcell* id){
	unsigned c = id->data.boolVal;
	if(c == 0) return "False";
	else return "True";
}

extern char* table_tostring (struct avm_memcell* id){
	if(id->data.strVal != NULL) return id->data.strVal;
	else {
		char* c = (char*)malloc(sizeof(char)* 50);
		sprintf(c, "%f", id->data.numval);
		return c;
	}
}

extern char* userfunc_tostring 	(struct avm_memcell* id){
	char* c = (char*)malloc(sizeof(char)* 40);
	sprintf(c, "user function - %d\n", id->data.funcVal);
	return c;
}

extern char* libfunc_tostring 	(struct avm_memcell* id){
	char* c = (char*)malloc(sizeof(char)* 40);
	sprintf(c, "library function - %s\n", id->data.libfuncVal);
	return c;
}

extern char* nil_tostring 		(struct avm_memcell* id){
	return "nil\n";
}

extern char* undef_tostring 	(struct avm_memcell* id){
	return "undef\n";
}




//CALLING FUNCTIONS
void execute_pusharg (struct instruction *instr){

	struct avm_memcell* arg = avm_translate_operand(instr->arg1, &ax);
	assert(arg);
	/*	This actuallly stack[top] = arg, but we have to use avm_assign	*/
	avm_assign(&stack[top], arg);
	++totalActuals;
	avm_dec_top();
}

void execute_newtable (struct instruction* instr){
    struct avm_memcell* lv = avm_translate_operand(instr->result, (struct avm_memcell*) 0);

    assert(lv && (&stack[0] <= lv && &stack[top] > lv || lv == &retval));

    avm_memcellclear(lv);

    lv->type = table_m;
    lv->data.tableVal = avm_tablenew();
    avm_tableincrefcounter(lv->data.tableVal);

}



void execute_tablegetelem(struct instruction* instr){
    struct avm_memcell* lv = avm_translate_operand(instr->result, (struct avm_memcell*) 0);
    struct avm_memcell* t = avm_translate_operand(instr->arg1, (struct avm_memcell*) 0);
    struct avm_memcell* i = avm_translate_operand(instr->arg2, &ax);

    assert(lv && (&stack[0] <= lv && &stack[top] > lv || lv == &retval));
    assert(t && &stack[0] <= t && &stack[top] > t);
    assert(i);

    avm_memcellclear(lv);
    lv->type = nil_m;       // default

    if(t->type != table_m)  {
		fprintf(stderr, "Error: illegal use of type %s as table!\n", typeStrings[t->type]);
		executionFinished = 1;

	}
    else{
		struct avm_memcell* content = avm_tablegetelem(t->data.tableVal, i);  //maybe wrong.
		if(content) avm_assign(lv, content);
		else{
			char* ts = avm_tostring(t);
			char* is = avm_tostring(i);
			fprintf(stderr, "Warning: %s[%s] not found!\n", ts, is);
			free(ts);
			free(is);
		}
    }
    
}

void execute_tablesetelem (struct instruction* instr) {
	struct avm_memcell* t = avm_translate_operand(instr->result, (struct avm_memcell*) 0);
	struct avm_memcell* i = avm_translate_operand(instr->arg1, &ax);
	struct avm_memcell* c = avm_translate_operand(instr->arg2, &bx);

	assert(t && &stack[0] <= t && &stack[top] > t);
	assert(i && c);

	if (t->type != table_m)
	{
		fprintf(stderr, "Error: illegal use of type %s as table!\n", typeStrings[t->type]);
	} else{
		avm_tablesetelem(t->data.tableVal, i, c);
	}
	
}



unsigned char avm_tobool (struct avm_memcell* m){
    assert (m->type >= 0 && m->type < undef_m);
    return (*toboolFuncs[m->type])(m);
}





unsigned char jeq_number 	(struct avm_memcell* rv1, struct avm_memcell* rv2){ return rv1->data.numval == rv2->data.numval; }
unsigned char jeq_string 	(struct avm_memcell* rv1, struct avm_memcell* rv2){ 
	if(strcmp(rv1->data.strVal,rv2->data.strVal) == 0)
		return 1;
	return 0; 
}
unsigned char jeq_bool 		(struct avm_memcell* rv1, struct avm_memcell* rv2){ }
unsigned char jeq_table 	(struct avm_memcell* rv1, struct avm_memcell* rv2){ }
unsigned char jeq_userfunc 	(struct avm_memcell* rv1, struct avm_memcell* rv2){ }
unsigned char jeq_libfunc 	(struct avm_memcell* rv1, struct avm_memcell* rv2){ }
unsigned char jeq_nil 		(struct avm_memcell* rv1, struct avm_memcell* rv2){ }
unsigned char jeq_undef 	(struct avm_memcell* rv1, struct avm_memcell* rv2){ }

typedef unsigned char (*equality_func_t)(struct avm_memcell* rv1, struct avm_memcell* rv2);

equality_func_t equalityFuncs[]={
	jeq_number 	,
	jeq_string 	,
	jeq_bool 	,
	jeq_table 	,
	jeq_userfunc,
	jeq_libfunc ,
	jeq_nil 	,
	jeq_undef
};

unsigned char avm_jeq (struct avm_memcell* rv1, struct avm_memcell* rv2){
	return (*equalityFuncs[rv1->type])(rv1,rv2);
}

void execute_jeq (struct instruction* instr) {
    assert(instr->result->type == label_a);

    struct avm_memcell* rv1 = avm_translate_operand(instr->arg1, &ax);
    struct avm_memcell* rv2 = avm_translate_operand(instr->arg2, &bx);

    unsigned char result = 0;

    if(rv1->type == undef_m || rv2->type == undef_m)        {
		executionFinished = 1;
		fprintf(stderr, "Error: undef involved in equality.\n");
	}
    else if (rv1->type == nil_m || rv2->type == nil_m)      result = rv1->type == nil_m && rv2->type == nil_m;
    else if (rv1->type == bool_m || rv2->type == bool_m)    result = (avm_tobool(rv1) == avm_tobool(rv2));
    else if (rv1->type != rv2->type)                        fprintf(stderr, "%s == %s is illegal!\n", typeStrings[rv1->type], typeStrings[rv2->type]);
    else  {
		result = avm_jeq(rv1,rv2);
	}//Equality check with dispatching
	

    if (!executionFinished && result){
        pc = instr->result->val;
    }
}

unsigned char jge_impl (double x, double y){ return((x>=y) ? 	1 : 0);	}
unsigned char jgt_impl (double x, double y){ return((x>y) ?  	1 : 0);		}
unsigned char jle_impl (double x, double y){ return((x<=y) ? 	1 : 0);		}
unsigned char jlt_impl (double x, double y){ return((x<y) ? 		1 : 0);		}



double add_impl (double x, double y) {return x + y;}
double sub_impl (double x, double y) {return x - y;}
double mul_impl (double x, double y) {return x * y;}
double div_impl (double x, double y) {
    if( y == 0 ) executionFinished = 1;
    else return x / y;
    return -1;
} 
double mod_impl (double x, double y) {
    if( y == 0 ) executionFinished = 1;
    return ((unsigned) x % (unsigned) y);
    return -1;
}

void execute_comparison (struct instruction* instr){

	//struct avm_memcell* lv = avm_translate_operand (instr->result, &cx);
    struct avm_memcell* rv1 = avm_translate_operand (instr->arg1, &ax);
    struct avm_memcell* rv2 = avm_translate_operand (instr->arg2, &bx);
	unsigned char c;

	// assert(lv && (&stack[AVM_STACKSIZE - 1] >= lv && lv > &stack[top] || lv == &retval));
    assert(rv1 && rv2);

	if (rv1->type != number_m || rv2->type != number_m){
		fprintf(stderr, "Error: not a number in comparison!\n");
        executionFinished = 1;
    }
    else {
        comparison_func_t op = comparisonFuncs[instr->opcode - jle_v];
        //avm_memcellclear(lv);
        c = (*op) (rv1->data.numval, rv2->data.numval);
		if(c == 1){
			pc = instr->result->val;
		}

    }
}


void execute_arithmetic (struct instruction* instr){

    struct avm_memcell* lv = avm_translate_operand (instr->result, &cx);
    struct avm_memcell* rv1 = avm_translate_operand (instr->arg1, &ax);
    struct avm_memcell* rv2 = avm_translate_operand (instr->arg2, &bx);

   // assert(lv && (&stack[AVM_STACKSIZE - 1] >= lv && lv > &stack[top] || lv == &retval));
    assert(rv1 && rv2);

    if (rv1->type != number_m || rv2->type != number_m){
		fprintf(stderr, "Error: not a number in arithmetic!\n");
        executionFinished = 1;
    }
    else {
        arithmetic_func_t op = arithmeticFuncs[instr->opcode - jne_v];
        avm_memcellclear(lv);
        lv->type        = number_m;
        lv->data.numval = (*op) (rv1->data.numval, rv2->data.numval);
    }
}


library_func_t avm_getlibraryfunc(char* id){

	if(strcmp(id, "print") == 0) { return (*libfuncs[0]);}
//	else if(strcmp(id, "input") == 0) { return (*libfuncs[1])();}
//	else if(strcmp(id, "objectmemberkeys") == 0) { return (*libfuncs[2])();}
//	else if(strcmp(id, "objecttotalmembers") == 0) { return (*libfuncs[3])();}
//	else if(strcmp(id, "objectcopy") == 0) { return (*libfuncs[4])();}
	else if(strcmp(id, "totalarguments") == 0) { return (libfuncs[2]);}
	else if(strcmp(id, "argument") == 0) { return (*libfuncs[3]);}
	else if(strcmp(id, "typeof") == 0) { return (*libfuncs[1]);}
//	else if(strcmp(id, "strtonum") == 0) { return (*libfuncs[8])();}
	else if(strcmp(id, "sqrt") == 0) { return (*libfuncs[4]);}
	else if(strcmp(id, "cos") == 0) { return (*libfuncs[5]);}
	else if(strcmp(id, "sin") == 0) { return (*libfuncs[6]);}
}


void avm_registerlibfunc(char* id, library_func_t addr){

		 if(strcmp(id, "print") == 0) { libfuncs[0] = addr;}
	else if(strcmp(id, "typeof") == 0) { libfuncs[1] = addr;}
/*	else if(strcmp(id, "objectmemberkeys") == 0) { libfuncs[2] = addr;}
	else if(strcmp(id, "objecttotalmembers") == 0) { libfuncs[3] = addr;}
	else if(strcmp(id, "objectcopy") == 0) { libfuncs[4] = addr;}
*/	else if(strcmp(id, "totalarguments") == 0) { libfuncs[2] = addr;}
	else if(strcmp(id, "argument") == 0) { libfuncs[3] = addr;}
/*	else if(strcmp(id, "input") == 0) { libfuncs[7] = addr;}
	else if(strcmp(id, "strtonum") == 0) { libfuncs[8] = addr;}
*/
	else if(strcmp(id, "sqrt") == 0) { libfuncs[4] = addr;}
	else if(strcmp(id, "cos") == 0) { libfuncs[5] = addr;}
	else if(strcmp(id, "sin") == 0) { libfuncs[6] = addr;}
	else assert(0);

}


void avm_initialize (void) {

	avm_initstack();

	avm_registerlibfunc("print", libfunc_print);
	avm_registerlibfunc("typeof", libfunc_typeof);
/*	avm_registerlibfunc("objectmemberkeys", libfunc_print);
	avm_registerlibfunc("objecttotalmembers", libfunc_print);
	avm_registerlibfunc("objectcopy", libfunc_print);
*/	avm_registerlibfunc("totalarguments", libfunc_totalarguments);
	avm_registerlibfunc("argument", libfunc_argument);
/*	avm_registerlibfunc("input", libfunc_print);
	avm_registerlibfunc("strtonum", libfunc_print);
*/
	avm_registerlibfunc("sqrt", libfunc_sqrt);
	avm_registerlibfunc("cos",  libfunc_cos);
	avm_registerlibfunc("sin",  libfunc_sin);

	/* Same for all the rest lib funcs. */
}

void avm_calllibfunc (char* id) {
	library_func_t f = avm_getlibraryfunc(id);
	
	if(!f){
		
		fprintf(stderr, "Error: Unsupported lib func '%s' called!\n", id);
		executionFinished = 1;
	}
	else {
		topsp = top;
		totalActuals = 0;
		(*f)();
		if(!executionFinished) execute_funcexit ((struct instruction*) 0);
	}
}


unsigned avm_totalactuals(){
	return avm_get_envalue(topsp + AVM_NUMACTUALS_OFFSET);
}

struct avm_memcell* avm_getactual (unsigned i){
	assert(i < avm_totalactuals());
	return &stack[topsp + AVM_STACKENV_SIZE + 1 + i];
}

void libfunc_print (){
	unsigned n = avm_totalactuals();
	unsigned i = 0;
	for(i; i<n; i++){
		char* s = avm_tostring(avm_getactual(i));
		puts(s);
		//free(s);
	}
}

void libfunc_typeof (void) {

	unsigned n = avm_totalactuals();

	if(n!=1){
		fprintf(stderr, "Error in libfunc typeof: one argument (not %d) expected in 'typeof'!\n", n);
	}else{
		/* 	Thats how a library func returns a result.
			It has to only set the 'retval' register!
		*/
		avm_memcellclear(&retval); /* Do not forget to clean-it up */
		retval.type = string_m;
		retval.data.strVal = strdup(typeStrings[avm_getactual(0)->type]);
	}
}

void libfunc_totalarguments (void){

	/* Get topsp of previous activations record */
	flag_totalargs = 1;

	unsigned p_topsp = avm_get_envalue(topsp + AVM_SAVEDTOPSP_OFFSET);
	avm_memcellclear(&retval);

	if (!p_topsp){ // if 0, no prev activation record
		fprintf(stderr, "Error in libfunc totalarguments: 'totalarguments' called outside a function!\n");
		retval.type = nil_m;
	} else{
		// Extract the number of actual arguments for the prev actovation record.
		retval.type = number_m;
		retval.data.numval = avm_get_envalue(p_topsp + AVM_NUMACTUALS_OFFSET);
	}
}


void libfunc_argument (void){
	unsigned p_topsp = avm_get_envalue(topsp + AVM_SAVEDTOPSP_OFFSET);
	avm_memcellclear(&retval);
	if (!p_topsp){ // if 0, no prev activation record
		fprintf(stderr, "Error in libfunc arguments: 'arguments' called outside a function!\n");
		retval.type = nil_m;
	} else{
		// Extract the number of actual arguments for the prev actovation record.
		retval.type = number_m;
		//retval.data.numval = avm_get_envalue(p_topsp + AVM_NUMACTUALS_OFFSET + i);
		retval = stack[p_topsp + AVM_NUMACTUALS_OFFSET +1];
		//retval = avm_getactual(i);
	}

}


void libfunc_sqrt (void){
	//unsigned p_topsp = avm_get_envalue(topsp + AVM_SAVEDTOPSP_OFFSET);

	//printf("avm_get_envalue = %d\n", p_topsp + AVM_NUMACTUALS_OFFSET +1);
	//retval = stack[p_topsp + AVM_NUMACTUALS_OFFSET +1];
	//if(){

	//}
	//printf(" retval type = %d\n", retval.type);
}

void libfunc_cos (void){
	
}

void libfunc_sin (void){
	
}

void execute_call (struct instruction* instr){
	
    struct avm_memcell* func = avm_translate_operand(instr->arg1, &ax);
    assert(func);
    avm_callsaveenviroment();

    switch(func->type){
        case userfunc_m:{
           // pc = func->data.funcVal;

		    pc = stack[user_ptr - func->data.funcVal].data.funcVal;
            assert(pc < AVM_ENDING_PC);
            assert(code[pc].opcode == funcenter_v);
            break;
        }
        case string_m: avm_calllibfunc(func->data.strVal); break;
        case libfunc_m: avm_calllibfunc(func->data.libfuncVal); break;
        default: {
            char* s = avm_tostring(func);
			fprintf(stderr, "Error in call: Cannot bind %s to function!\n", s);
            free(s);
            executionFinished = 1;
        }
    }
}

void avm_memcellclear (struct avm_memcell* m){
	if(m->type != undef_m){
		memclear_func_t f = memclearFuncs[m->type];
		if(f) (*f)(m);
		m->type = undef_m;
	}

}



void avm_assign (struct avm_memcell* lv, struct avm_memcell* rv){
	if(lv == rv) return;	// Same cells?? Destructive to assign!!

    if(lv->type == table_m && rv->type == table_m && lv->data.tableVal == rv->data.tableVal) return;
	
	if(rv->type == undef_m) fprintf(stderr, "%s", "Warning: assigning from 'undef' content!\n");

    avm_memcellclear(lv);

    memcpy(lv, rv, sizeof(struct avm_memcell));

    if(lv->type == string_m) lv->data.strVal = strdup(rv->data.strVal);
    else if(lv->type == table_m) avm_tableincrefcounter (lv->data.tableVal);
	else if(lv->type == bool_m){
		if(rv->data.boolVal == 1){
			lv->data.boolVal = 1;
		}else{
			lv->data.boolVal = 0;
		}
	}  


}

char* avm_tostring (struct avm_memcell* m){
	assert(m->type >= 0 && m->type <= undef_m);
	return (*tostringFuncs[m->type]) (m);
}

void execute_assign(struct instruction* instr){


	struct avm_memcell* lv = avm_translate_operand(instr->arg2, (struct avm_memcell*) 0);
    struct avm_memcell* rv = avm_translate_operand(instr->arg1, &ax);

    assert(lv && (&stack[AVM_STACKSIZE] >= lv && lv > &stack[top] || lv == &retval)); // N what
    assert(rv); // should do similar assertion tests here.


    avm_assign(lv, rv);

}


void pre_exec (){
	top = AVM_STACKSIZE - programVarOffset;
	globalmem = AVM_STACKSIZE;
	string_ptr = top;
	topsp = top;
}

void expand_code(void){
	struct instruction* p = (struct instruction*) malloc(total*sizeof(struct instruction));
	if(code){
		memcpy(p, code, CURR_SIZE);
		free(code);
	}
	code = p;
}

void print_code () {

	int i = 0;
	struct instruction* p;
	char* op;
	printf("\nIn print code total = %d\n", code_counter);
	for(i; i<code_counter; i++){
		p = code+i;
		op = vmopcode_to_string(p->opcode);


		// Console
		printf("%d: %s ", p->srcLine, op);

		if(p->arg1) printf("arg1::%d - %d, ", 		p->arg1->type,   p->arg1->val);
		if(p->arg2) printf("arg2::%d - %d, ", 		p->arg2->type,   p->arg2->val);
		if(p->result) printf("result::%d - %d, ", 	p->result->type, p->result->val);

		printf("\n");
	}

	
}

void read_binary (){

	char* c = (char*)malloc(sizeof(char)*2);
	char* buffer = (char*)malloc(sizeof(char)*10000);
	char* buffer2 = (char*)malloc(sizeof(char)*10000);
	int i = 0;
	int k = 0;
	double d = 0;
	unsigned l = 0;
	int instr_flag = 0;
	int total = 0;
	struct instruction* instr;

	// Magic Number
	fread(buffer, sizeof(char), 7, fpb);
	int a = atoi(buffer);
	assert(a == 6942069 && "Error: Program is not of alpha language\n");
	a = 0;
	fread(c, sizeof(char), 1, fpb);
	strcpy(buffer, "");



	// Strings
		// total
	fread(c, sizeof(char), 1, fpb);
	while (strcmp(c, "|") != 0){
		strcat(buffer, c);
		fread(c, sizeof(char), 1, fpb);
	}

	total = atoi(buffer);


	strcpy(buffer, "");
	
	// Size
	for(k; k< total; k++ ){
		fread(c, sizeof(char), 1, fpb);
		while (strcmp(c, ",") != 0){
			strcat(buffer, c);
			fread(c, sizeof(char), 1, fpb);
		}

		a = atoi(buffer);
		free(buffer);
		buffer = (char*)malloc(sizeof(char)*10000);
		memset(buffer,0,strlen(buffer));
		
		//Strings
		fread(buffer, sizeof(char), a, fpb);
		stack[top].data.strVal = (char*)malloc(sizeof(char)*strlen(buffer)+5);
		strcpy(stack[top].data.strVal, buffer);

		stack[top].type = string_m;
		top--;
		i = 0;
		strcpy(buffer, "");

	}
	fread(c, sizeof(char), 1, fpb);

	memset(buffer,0,strlen(buffer));
	i = 0;
	k = 0;

	num_ptr = top;

	// Numbers
	// total
	fread(c, sizeof(char), 1, fpb);
	while (strcmp(c, "|") != 0){
		strcat(buffer, c);
		fread(c, sizeof(char), 1, fpb);
	}


	total = atoi(buffer);
	memset(buffer,0,strlen(buffer));

	// number

	for(k; k<total; k++){
		fread(c, sizeof(char), 1, fpb);
		while (strcmp(c, ",") != 0){
			strcat(buffer, c);
			fread(c, sizeof(char), 1, fpb);
		}
		d = atof(buffer);
		stack[top].data.numval = d;
		stack[top].type = number_m;
		top--;
		strcpy(buffer, "");
		memset(buffer,0,strlen(buffer));

	}

	fread(c, sizeof(char), 1, fpb);

	k = 0;

	user_ptr = top;

	// Userfuncs
	// total
	fread(c, sizeof(char), 1, fpb);
	while (strcmp(c, "|") != 0){
		strcat(buffer, c);
		fread(c, sizeof(char), 1, fpb);
	}


	total = atoi(buffer);
	memset(buffer,0,strlen(buffer));

	// actual funcs

	for(k; k<total; k++){
		stack[top].type = userfunc_m;

		// address
		fread(c, sizeof(char), 1, fpb);
		while (strcmp(c, ",") != 0){
			strcat(buffer, c);
			fread(c, sizeof(char), 1, fpb);
		}
		a = atoi(buffer);
		stack[top].data.funcVal = a;
		
		strcpy(buffer, "");
		memset(buffer,0,strlen(buffer));

		// Local size
		fread(c, sizeof(char), 1, fpb);
		while (strcmp(c, ",") != 0){
			strcat(buffer, c);
			fread(c, sizeof(char), 1, fpb);
		}
		a = atoi(buffer);
		stack[top].localSize = a;
		strcpy(buffer, "");
		memset(buffer,0,strlen(buffer));

		// id
		fread(c, sizeof(char), 1, fpb);
		while (strcmp(c, ",") != 0){
			strcat(buffer, c);
			fread(c, sizeof(char), 1, fpb);
		}

		a = atoi(buffer);
		free(buffer);
		buffer = (char*)malloc(sizeof(char)*10000);
		memset(buffer,0,strlen(buffer));
		
		fread(buffer, sizeof(char), a, fpb);
		top--;
		i = 0;
		strcpy(buffer, "");

		memset(buffer,0,strlen(buffer));

		fread(c, sizeof(char), 1, fpb);



	}

	fread(c, sizeof(char), 1, fpb);

	k = 0;

	lib_ptr = top;


	// Lib Funcs

		fread(c, sizeof(char), 1, fpb);
	while (strcmp(c, "|") != 0){
		strcat(buffer, c);
		fread(c, sizeof(char), 1, fpb);
	}


	total = atoi(buffer);
	num_of_lib_funcs = total;
	memset(buffer,0,strlen(buffer));


	for(k; k< total; k++ ){
		fread(c, sizeof(char), 1, fpb);
		while (strcmp(c, ",") != 0){
			strcat(buffer, c);
			fread(c, sizeof(char), 1, fpb);
		}

		a = atoi(buffer);
		free(buffer);
		buffer = (char*)malloc(sizeof(char)*10000);
		memset(buffer,0,strlen(buffer));
		
		//Strings
		fread(buffer, sizeof(char), a, fpb);
		i = 0;
		stack[top].data.libfuncVal = (char*)malloc(sizeof(char)*strlen(buffer));
		strcpy(stack[top].data.libfuncVal, buffer);
		stack[top].type = libfunc_m;
		top--;
		i = 0;
		strcpy(buffer, "");

		fread(c, sizeof(char), 1, fpb);

	}
	fread(c, sizeof(char), 1, fpb);

	memset(buffer,0,strlen(buffer));
	i = 0;
	k = 0;

	// Instructions
	// Total
	fread(c, sizeof(char), 1, fpb);
	while (strcmp(c, "|") != 0){
		strcat(buffer, c);
		fread(c, sizeof(char), 1, fpb);
	}


	total = atoi(buffer);
	memset(buffer,0,strlen(buffer));

	expand_code();

	for(i=0; i<total; i++){

		

		
		instr = code + code_counter++;


		// label
		fread(c, sizeof(char), 1, fpb);
		while (strcmp(c, ",") != 0){
			strcat(buffer, c);
			fread(c, sizeof(char), 1, fpb);
		}

		l = atoi(buffer);
		instr->srcLine = l;
		memset(buffer,0,strlen(buffer));

		


		// Type
		fread(c, sizeof(char), 1, fpb);
		while (strcmp(c, ",") != 0){
			strcat(buffer, c);
			fread(c, sizeof(char), 1, fpb);
		}


		if(strcmp(buffer, "assign") == 0) 				{instr->opcode = assign_v; 	}
		else if (strcmp(buffer, "add") == 0) 			{instr->opcode = add_v; 	}
		else if (strcmp(buffer, "sub") == 0)			{instr->opcode = sub_v; 	}
		else if (strcmp(buffer, "mul") == 0)			{instr->opcode = mul_v; 	}
		else if (strcmp(buffer, "div") == 0)			{instr->opcode = div_v; 	}
		else if (strcmp(buffer, "not") == 0)			{instr->opcode = not_v; 	}
		else if (strcmp(buffer, "jeq") == 0)			{instr->opcode = jeq_v; 	}
		else if (strcmp(buffer, "jne") == 0)			{instr->opcode = jne_v; 	}
		else if (strcmp(buffer, "jle") == 0)			{instr->opcode = jle_v; 	}
		else if (strcmp(buffer, "jge") == 0)			{instr->opcode = jge_v; 	}
		else if (strcmp(buffer, "jlt") == 0)			{instr->opcode = jlt_v; 	}
		else if (strcmp(buffer, "jgt") == 0)			{instr->opcode = jgt_v; 	}
		else if (strcmp(buffer, "call") == 0) 			{instr->opcode = call_v; 	}
		else if (strcmp(buffer, "pusharg") == 0) 		{instr->opcode = pusharg_v; }
		else if (strcmp(buffer, "funcenter") == 0) 		{instr->opcode = funcenter_v; 	}
		else if (strcmp(buffer, "funcexit") == 0) 		{instr->opcode = funcexit_v; 	}
		else if (strcmp(buffer, "newtable") == 0) 		{instr->opcode = newtable_v; 	}
		else if (strcmp(buffer, "tablegetelem") == 0) 	{instr->opcode = tablegetelem_v; }
		else if (strcmp(buffer, "tablesetelem") == 0) 	{instr->opcode = tablesetelem_v; }
		else if (strcmp(buffer, "nop") == 0)				{instr->opcode = nop_v; 	}
		else if (strcmp(buffer, "jump") == 0) 			{instr->opcode = jump_v; 		}




		memset(buffer,0,strlen(buffer));

		// arg1
		fread(c, sizeof(char), 1, fpb);
		while (strcmp(c, "-") != 0){
			strcat(buffer, c);
			fread(c, sizeof(char), 1, fpb);
		}

		instr->arg1 = (struct vmarg*)malloc(sizeof(struct vmarg));

		if(strcmp(buffer, ",") != 0 && strcmp(buffer, "|") != 0){
			l = atoi(buffer);
			instr->arg1->type = (enum vmarg_t) l;

			memset(buffer,0,strlen(buffer));

			fread(c, sizeof(char), 1, fpb);
			while (strcmp(c, ",") != 0 && strcmp(c, "|") != 0){

				strcat(buffer, c);
				fread(c, sizeof(char), 1, fpb);
			}
			if(strcmp(c, "|") == 0) {
				instr_flag = 1;
			}
			l = atoi(buffer);

			instr->arg1->val = l;



		}
		else if(strcmp(buffer, "|") == 0) instr_flag = 1;

		memset(buffer,0,strlen(buffer));

		// arg2
		if(instr_flag == 0){
			fread(c, sizeof(char), 1, fpb);
			while (strcmp(c, "-") != 0){
				strcat(buffer, c);
				fread(c, sizeof(char), 1, fpb);
			}
			strcat(buffer, c);

			instr->arg2 = (struct vmarg*)malloc(sizeof(struct vmarg));

			if(strcmp(buffer, ",") != 0 && strcmp(buffer, "|") != 0){

				l = atoi(buffer);
				instr->arg2->type = (enum vmarg_t) l;

				memset(buffer,0,strlen(buffer));

				fread(c, sizeof(char), 1, fpb);
				while (strcmp(c, ",") != 0 && strcmp(c, "|") != 0){

					strcat(buffer, c);
					fread(c, sizeof(char), 1, fpb);
				}
				if(strcmp(c, "|") == 0) {
					instr_flag = 1;
				}
				l = atoi(buffer);
				instr->arg2->val = l;
				}
				memset(buffer,0,strlen(buffer));

		}
		else if(strcmp(buffer, "|") == 0) {
					instr_flag == 1;
		}

		// result
		if(instr_flag == 0){
			fread(c, sizeof(char), 1, fpb);
			while (strcmp(c, "-") != 0){
				strcat(buffer, c);
				fread(c, sizeof(char), 1, fpb);
			}
			strcat(buffer, c);

			instr->result = (struct vmarg*)malloc(sizeof(struct vmarg));

			if(strcmp(buffer, ",") != 0){
				l = atoi(buffer);
				instr->result->type = (enum vmarg_t) l;

				memset(buffer,0,strlen(buffer));

				fread(c, sizeof(char), 1, fpb);
				while (strcmp(c, "|") != 0){
					strcat(buffer, c);
					fread(c, sizeof(char), 1, fpb);
				}
				l = atoi(buffer);
				instr->result->val = l;

			}

			memset(buffer,0,strlen(buffer));


		}

		instr_flag = 0;
		printf("\n");
	}

	topsp = top;




}


void print_stack(){

	int i = AVM_STACKSIZE -1;
	printf("-----------------------------------------------------------------------------\n");
	printf("Globals of memStack\n");

	for(i; i>string_ptr; i--){
		printf("%d:  type %d   ", i, stack[i].type);
		if(stack[i].type == 0) 			printf("number = %f\n", stack[i].data.numval);
		else if (stack[i].type == 1) 	printf("string = %s\n", stack[i].data.strVal);
		else if (stack[i].type == 2)	printf("bool = %d\n", stack[i].data.boolVal);
		else if (stack[i].type == 4)	printf("Local size = %d,  addr = %d,\n", stack[i].localSize, stack[i].data.funcVal);
		else if (stack[i].type == 5)	printf("library name = %s\n", stack[i].data.libfuncVal);
		else 							printf("Nothing from the above?\n");
	}
	printf("Strings of memStack\n");
	for(i; i>num_ptr; i--){
		printf("%d:  type %d   ", i, stack[i].type);
		printf("string = %s\n", stack[i].data.strVal);
	}
	printf("Nums of memStack\n");
	for(i; i>user_ptr; i--){
		printf("%d:  type %d   ", i, stack[i].type);
		printf("number = %f\n", stack[i].data.numval);
	}
	printf("User funcs of memStack\n");
	for(i; i>lib_ptr; i--){
		printf("%d:  type %d   ", i, stack[i].type);
		printf("Local size = %d,  addr = %d,\n", stack[i].localSize, stack[i].data.funcVal);
	}
	printf("Lib funcs of memStack\n");
	for(i; i>(lib_ptr-num_of_lib_funcs); i--){
		printf("%d:  type %d   ", i, stack[i].type);
		printf("library name = %s\n", stack[i].data.libfuncVal);
	}
	printf("activation records \n");
	for(i; i>top; i--){
		printf("%d:  type %d   ", i, stack[i].type);

		if(stack[i].type == 0) 			printf("number = %f\n", stack[i].data.numval);
		else if (stack[i].type == 1) 	printf("string = %s\n", stack[i].data.strVal);
		else if (stack[i].type == 2)	printf("bool = %d\n, ", stack[i].data.boolVal);
		else if (stack[i].type == 4)	printf("Local size = %d,  addr = %d,\n", stack[i].localSize, stack[i].data.funcVal);
		else if (stack[i].type == 5)	printf("library name = %s\n", stack[i].data.libfuncVal);
		else 							printf("Nothing from the above?\n");
	}

	/*for (i; i>top; i--){
		printf("%d:  type %d   ", i, stack[i].type);

		if(stack[i].type == 0) 			printf("number = %f\n", stack[i].data.numval);
		else if (stack[i].type == 1) 	printf("string = %s\n", stack[i].data.strVal);
		else if (stack[i].type == 4)	printf("Local size = %d,  addr = %d,\n", stack[i].localSize, stack[i].data.funcVal);
		else if (stack[i].type == 5)	printf("library name = %s\n", stack[i].data.libfuncVal);
		else 							printf("Nothing from the above?\n");
	}*/
	printf("POINTERS: string = %d, num = %d, userfunc = %d, libfunc = %d\n", string_ptr, num_ptr, user_ptr, lib_ptr);
	printf("-----------------------------------------------------------------------------\n");
}
// TODO get number string lib func.

void memclear_string (struct avm_memcell* m){
	assert(m->data.strVal);
	free(m->data.strVal);
}

void memclear_table (struct avm_memcell* m){
	assert(m->data.tableVal);
	free(m->data.tableVal);
}


void avm_dec_top (){
	if(!top) {
		//Stack Overflow. :)
		fprintf(stderr, "Error: Stack Overflow.\n");
		executionFinished = 1;
	}
	else top--;
}


void avm_push_envalue (unsigned val){
	stack[top].type = number_m;
	stack[top].data.numval = val;
	avm_dec_top();
}



void execute_circle (){
	if(executionFinished) {				//crash
		printf("------------------------------------------------------------------\n");
		printf("Program crashed.\n");
		printf("------------------------------------------------------------------\n");
		printf("Created by: \ni) tOhNyAaAaA (VS Code Hot Tub Streamer)\nii) RISC-I Expert (Data Structure Ninja)\niii) Autos pou pire katalathos epilogis Katebaini kai thelei na parei kai ptixio\n");
		return;
	}
	else if (pc == AVM_ENDING_PC){		//finish
		printf("------------------------------------------------------------------\n");
		printf("Program finished.\n");
		printf("------------------------------------------------------------------\n");
		printf("Created by: \ni) tOhNyAaAaA (VS Code Hot Tub Streamer)\nii) RISC-I Expert (Data Structure Ninja)\niii) Autos pou pire katalathos epilogis Katebaini kai thelei na parei kai ptixio\n");
		executionFinished = 1;
		return;
	}
	else{
		
		assert(pc < AVM_ENDING_PC);
		struct instruction* instr = code + pc;
		if(instr->opcode == 24) {
			pc = instr->arg1->val;
			return;
		}
		assert(
			instr->opcode >= 0 &&
			instr->opcode <= AVM_MAX_INSTRUCTIONS
		);
		if (instr->srcLine) currLine = instr->srcLine;
		unsigned oldPC = pc;
		(*executeFuncs[instr->opcode])(instr);
		if(pc == oldPC) pc++;

	}
}



void avm_callsaveenviroment (){
	avm_push_envalue(totalActuals);
	avm_push_envalue(pc+1);
	avm_push_envalue(top + totalActuals + 2);
	avm_push_envalue (topsp);
}

unsigned avm_get_envalue (unsigned i){
	unsigned val = (unsigned) stack[i].data.numval;
	if (flag_totalargs==1){
		if(stack[i].type != number_m || stack[i].data.numval != ((double) val)){
			flag_totalargs == 0;
			return 0;
		}
	}
	assert(stack[i].type == number_m);
	assert(stack[i].data.numval == ((double) val));
	return val;
}

void execute_funcexit (struct instruction* unused) {
	unsigned oldTop = top;
	// TODO avm_get
	top = avm_get_envalue (topsp + AVM_SAVEDTOP_OFFSET);
	pc  = avm_get_envalue (topsp + AVM_SAVEDPC_OFFSET);
	topsp = avm_get_envalue (topsp + AVM_SAVEDTOPSP_OFFSET);
	while (oldTop++ <= top) avm_memcellclear(&stack[oldTop]);
}



void execute_funcenter (struct instruction* instr){
	
	totalActuals = 0;
	struct userfunc* funcInfo = (struct userfunc*)malloc(sizeof(struct userfunc));
	funcInfo->localSize = stack[user_ptr -instr->arg1->val].localSize;
	funcInfo->address = pc;
	
	stack[user_ptr -instr->arg1->val].data.funcVal = pc;
	


	topsp = top;
	top = top - funcInfo->localSize;


}



double consts_getnumber (unsigned val){
	return stack[num_ptr - val].data.numval;
}

char* consts_getstring(unsigned val){
	return stack[string_ptr - val].data.strVal;
}

char* libfuncs_getused(unsigned val){
	return stack[lib_ptr - val].data.libfuncVal;
}

struct avm_memcell* avm_translate_operand (struct vmarg* arg, struct avm_memcell* reg){



	switch(arg->type){

		//Variables
		case global_a:	return &stack[AVM_STACKSIZE-1-arg->val];
		case local_a:	return &stack[topsp-arg->val];
		case formal_a:	return &stack[topsp+AVM_STACKENV_SIZE+1+arg->val];
		case retval_a:	return &retval;

		// Constants
		case number_a:	{
			reg->type = number_m;
			reg->data.numval = consts_getnumber(arg->val);
			return reg;
		}
		case string_a:	{
			reg->type = string_m;
			reg->data.strVal = strdup(consts_getstring(arg->val));
			return reg;
		}
		case bool_a:	{
			reg->type = bool_m;
			reg->data.boolVal = arg->val;
			return reg;
		}
		case nil_a:		{
			reg->type = nil_m;
			return reg;
		}

		// Functions
		case userfunc_a: {
			reg->type = userfunc_m;
			reg->data.funcVal = arg->val;
			return reg;
		}
		case libfunc_a:	{
			reg->type = libfunc_m;
			reg->data.libfuncVal = libfuncs_getused(arg->val);
			return reg;
		}
	}

}



//---------------------PHASE 4 FUNCS -----------------------------------

//7: jump 9

//9: assign

//taddress to 9 sto jump
//iaddress to 9 quad.



void print_instructions (){

	int i = 0;
	int k = 0;
	int u = 0;
	int sizee;
	char* src;
	char* op;
	char* line = (char*)malloc(sizeof(char)* 10000);
	src = (char*)malloc(sizeof(char)*50);

	// Magic number.
	
	sprintf(src, "%d|", 6942069);
	strcat(line, src);
	while(line[k]){
		fwrite(&line[k], 1, sizeof(line[k]),fpb);
		k++;
	}
	
	k = 0;
	strcpy(line, "");
	// Const strings.
	sprintf(src, "%d|", totalStringConsts);
	strcat(line, src);
	for(u; u<totalStringConsts; u++){
		sizee = strlen(stringConsts[u]);
		sprintf(src, "%d,", sizee);
		strcat(line, src);
		sprintf(src, "%s", stringConsts[u]);
		strcat(line, src);
	}
	strcat(line, "|");

	while(line[k]){
		fwrite(&line[k], 1, sizeof(line[k]),fpb);
		k++;
		
	}

	u = 0;
	k = 0;
	free(line);

	line = (char*)malloc(sizeof(char)* 10000);
	strcpy(line, "");

	// const nums.
	
	sprintf(src, "%d|", totalNumConsts);
	strcat(line, src);
	
	for(u; u<totalNumConsts; u++){
		sprintf(src, "%f,", numConsts[u]);
		strcat(line, src);
	}
	strcat(line, "|");

	while(line[k]){
		fwrite(&line[k], 1, sizeof(line[k]),fpb);
		k++;
	}

	u = 0;
	k = 0;
	free(line);

	line = (char*)malloc(sizeof(char)* 10000);
	strcpy(line, "");
	


	// User funcs
	sprintf(src, "%d|", totalUserFuncs);
	strcat(line, src);
	struct userfunc* tmp = userFuncs;
	for(u; u<totalUserFuncs; u++){

		//address
		sprintf(src, "%d,", tmp->address);
		strcat(line, src);
		// LocalSize
		sprintf(src, "%d,", tmp->localSize);
		strcat(line, src);
		
		// id
		sizee = strlen(tmp->id);
		sprintf(src, "%d,", sizee);
		strcat(line, src);
		sprintf(src, "%s,", tmp->id);
		strcat(line, src);
		

		tmp = tmp->next;

	}
	strcat(line, "|");

	while(line[k]){
		fwrite(&line[k], 1, sizeof(line[k]),fpb);
		k++;
	}

	u = 0;
	k = 0;
	free(line);
	line = (char*)malloc(sizeof(char)* 10000);
	strcpy(line, "");
	strcpy(src, "");

	// LibFuncs
	sprintf(src, "%d|", totalNamedLibfuncs);
	strcat(line, src);
	for(u; u<totalNamedLibfuncs; u++){
		sizee = strlen(namedLibFuncs[u]);
		sprintf(src, "%d,", sizee);
		strcat(line, src);
		sprintf(src, "%s,", namedLibFuncs[u]);
		strcat(line, src);
	}
	strcat(line, "|");

	while(line[k]){
		
		fwrite(&line[k], 1, sizeof(line[k]),fpb);
		k++;
	}

	u = 0;
	k = 0;
	free(line);
	line = (char*)malloc(sizeof(char)* 10000);
	strcpy(line, "");




	// Instructions

	src = (char*)malloc(sizeof(char));
	sprintf(src, "%d|", currInstruction);
	strcat(line, src);
	free(src);

	for(i; i<currInstruction; i++){
		op = vmopcode_to_string(instructions[i].opcode);


		// Console
		printf("=%d: %s ", instructions[i].srcLine, op);

		if(instructions[i].arg1) printf("arg1::%d - %d, ", instructions[i].arg1->type, instructions[i].arg1->val);
		if(instructions[i].arg2) printf("arg2::%d - %d, ", instructions[i].arg2->type, instructions[i].arg2->val);
		if(instructions[i].result) printf("result::%d - %d, ", instructions[i].result->type, instructions[i].result->val);

		printf("\n");

		// File
		fprintf(fp2, "=%d: %s ", instructions[i].srcLine, op);

		if(instructions[i].arg1)   fprintf(fp2, "arg1::%d - %d, ", instructions[i].arg1->type, instructions[i].arg1->val);
		if(instructions[i].arg2)   fprintf(fp2, "arg2::%d - %d, ", instructions[i].arg2->type, instructions[i].arg2->val);
		if(instructions[i].result) fprintf(fp2, "result::%d - %d, ", instructions[i].result->type, instructions[i].result->val);

		fprintf(fp2, "\n");

		// Binary

		

		src = (char*)malloc(sizeof(char));
		sprintf(src, "%d,", instructions[i].srcLine);
		strcat(line, src);
		free(src);

		src = (char*)malloc(sizeof(char)* strlen(op)+1);
		strcat(src, op);
		strcat(src, ",");
		
		strcat(line, src);




		if(instructions[i].arg1)  {
			sprintf(src, "%d-", instructions[i].arg1->type);
			strcat(line, src);
			sprintf(src, "%d", instructions[i].arg1->val);
			strcat(line, src);
			if (instructions[i].arg2 || instructions[i].result) strcat(line, ",");
		}
		if(instructions[i].arg2)   {
			sprintf(src, "%d-", instructions[i].arg2->type);
			strcat(line, src);
			sprintf(src, "%d", instructions[i].arg2->val);
			strcat(line, src);
			if (instructions[i].result) strcat(line, ",");
		}
		if(instructions[i].result) {
			sprintf(src, "%d-", instructions[i].result->type);
			strcat(line, src);
			sprintf(src, "%d", instructions[i].result->val);
			strcat(line, src);
		}
		strcat(line, "|");
/*
		src = "\n";
		fwrite(src,sizeof(src),1,fpb);

		*/
	} 
	k = 0;

	while(line[k]){
		fwrite(&line[k], 1, sizeof(line[k]),fpb);
		k++;
	}
	
}



void append(struct incomplete_jump* ret, int i){

	struct incomplete_jump* node = (struct incomplete_jump*)malloc(sizeof(struct incomplete_jump));
	struct incomplete_jump* tmp;
	node->instrNo = i;

	if(ret == NULL){
		ret = node;
	}
	else{
		tmp = ret;
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = node;
	}
}

void push_funcstack(SymbolTableEntry* node){

	struct func_stack* tmp = (struct func_stack*)malloc(sizeof(struct func_stack));
	tmp->sym = node;

	if(funcstack == NULL){
		funcstack = tmp;
	}
	else{
		tmp->next = funcstack;
		funcstack = tmp;
	}
}

 struct func_stack* pop_funcstack(){
	struct func_stack* tmp = (struct func_stack*)malloc(sizeof(struct func_stack));
	funcstack = tmp->next;
	return tmp;
}

void add_incomplete_jump (unsigned instrNo, unsigned iaddress){

	struct incomplete_jump* node = (struct incomplete_jump*)malloc(sizeof(struct incomplete_jump));
	struct incomplete_jump* tmp;
	node->iaddress = iaddress;
	node->instrNo = instrNo;
	node->next = NULL;

	if(ij_total == 0){
		ij_head = node;
	}
	else {
		tmp = ij_head;
		while(tmp->next != NULL){
			tmp->next;
		}
		tmp->next = node;

	}
	ij_total++;
}
void expand_consts_string(void){
	assert(max_const_string == totalStringConsts);
	char** p = (char**) malloc(NEW_SIZE);
	if(stringConsts){
		memcpy(p, stringConsts, CURR_SIZE);
		free(stringConsts);
	}
	stringConsts = p;
	max_const_string +=EXPAND_SIZE;
}
unsigned consts_newstring (char* s){
	if(totalStringConsts == max_const_string){
		expand_consts_string();
	}

	stringConsts[totalStringConsts] = s;
	totalStringConsts++;

	return totalStringConsts-1;
}

void expand_consts_num(void){
	assert(max_const_num == totalNumConsts);
	double* p = (double*) malloc(NEW_SIZE);
	if(numConsts){
		memcpy(p, numConsts, CURR_SIZE);
		free(numConsts);
	}
	numConsts = p;
	max_const_num +=EXPAND_SIZE;
}
unsigned consts_newnumber (double n){
	if(totalNumConsts == max_const_num){
		expand_consts_num();
	}
	
	numConsts[totalNumConsts] = n;
	totalNumConsts++;

	return totalNumConsts - 1;
}
void expand_consts_libs(void){
	assert(max_libs == totalNamedLibfuncs);
	char** p = (char**) malloc(NEW_SIZE);
	if(namedLibFuncs){
		memcpy(p, namedLibFuncs, CURR_SIZE);
		free(namedLibFuncs);
	}
	namedLibFuncs = p;
	max_libs +=EXPAND_SIZE;
}
unsigned libfuncs_newused (char* s){
	if(totalNamedLibfuncs == max_libs){
		expand_consts_libs();
	}
	namedLibFuncs[totalNamedLibfuncs] = s;
	totalNamedLibfuncs++;

	return totalNamedLibfuncs-1;
}
void expand_consts_user(void){
	assert(max_user_funcs == totalUserFuncs);
	struct userfunc* p = (struct userfunc*) malloc(sizeof(struct userfunc));
	if(userFuncs){
		memcpy(p, userFuncs, CURR_SIZE);
		free(userFuncs);
	}
	userFuncs = p;
	max_user_funcs +=EXPAND_SIZE;
}

/*
unsigned userfuncs_newfunc (SymbolTableEntry* sym){
	char* str1 = "_";


	// search hidden

	// search
	// flag = 0
	// flag = 1


	// funcstart -> push			func flag = 0,
	// funcend -> pop				func_flag = 1,
	// call kanoniko -> search  	strncmp(str1, sym->name, 1) != 0
	// call hidden -> allo search 	strncmp(str1, sym->name, 1) == 0
	

	// call hidden -> allo search 	strncmp(str1, sym->name, 1) == 0
	// Needs fix.
	/*if(strncmp(str1, sym->name, 1) == 0){
		struct userfunc* tmpp = (struct userfunc*)malloc(sizeof(struct userfunc));
		tmpp= userFuncs;
		int count = 0;

		if(totalUserFuncs != 0){
			printf("IN SEARCH IN USERFUNCS\n");
			while (tmpp){
				if(strcmp(tmpp->id, sym->name) ==0){
					printf("Found function %s with local size %d\n", tmpp->id, tmpp->localSize);
					return count;
				}
				tmpp = tmpp->next;
				count++;
			}
		}
	}
	// Calls, search in list.

	if(func_start_end_flag == 0 && strncmp(str1, sym->name, 1) != 0){
		printf("in userfunc\n");
		if(totalUserFuncs == max_user_funcs){
			printf("Before expand user\n");
			expand_consts_user();
		}
		struct userfunc* node;
		struct userfunc* tmp;
		int i=0;
		node = (struct userfunc*)malloc(sizeof(struct userfunc));
		node->id = sym->name;
		node->localSize = sym->totalLocals;
		node->next = NULL;

		struct userfunc* tmpp = (struct userfunc*)malloc(sizeof(struct userfunc));
		tmpp= userFuncs;
		int count = 0;

	/*	if(totalUserFuncs != 0){
			printf("IN SEARCH IN USERFUNCS\n");
			while (tmpp){
				if(strcmp(tmpp->id, sym->name) ==0){
					return count;
				}
				tmpp = tmpp->next;
				count++;
			}
		}

		if(totalUserFuncs == 0){
			printf("First if\n");
			userFuncs = node;
			struct func_start_end* tmp = (struct func_start_end*)malloc(sizeof(struct func_start_end));
			tmp->label = totalUserFuncs;

			userFuncs_stack = tmp;
		}
		else{
			printf("First else\n");
			tmp = userFuncs;

			for(i; i<=totalUserFuncs; i++){
				tmp->next;
			}
			tmp->next = node;

			struct func_start_end* tmp2 = (struct func_start_end*)malloc(sizeof(struct func_start_end));
			tmp2->label = totalUserFuncs;

			if(userFuncs_stack == NULL){
				userFuncs_stack = tmp2;
			}
			else{
				tmp2->next = userFuncs_stack;
				userFuncs_stack = tmp2;
			}
		}
		totalUserFuncs++;
		return totalUserFuncs-1;
	}
	else if (strncmp(str1, sym->name, 1) == 0){


		struct userfunc* tmpp = (struct userfunc*)malloc(sizeof(struct userfunc));
		tmpp= userFuncs;
		int count = 0;

		if(totalUserFuncs != 0){
			printf("IN SEARCH IN USERFUNCS\n");
			while (tmpp){
				if(strcmp(tmpp->id, sym->name) ==0){
					printf("Found function %s with local size %d\n", tmpp->id, tmpp->localSize);
					return count;
				}
				tmpp = tmpp->next;
				count++;
			}
		}

	}
	else if (func_start_end_flag == 1){
		printf("in 3rd search sym = %s\n", sym->name);
		struct userfunc* tmpp = (struct userfunc*)malloc(sizeof(struct userfunc));
		tmpp= userFuncs;
		int count = 0;
		if(totalUserFuncs != 0){
			printf("IN SEARCH IN USERFUNCS\n");
			printf("zzzz name = %s\n", tmpp->id);
			while (tmpp){
				printf("zzzz name = %s, sym = %s\n", tmpp->id, sym->name);
				if(strcmp(tmpp->id, sym->name) == 0){
					printf("Found function %s with local size %d\n", tmpp->id, tmpp->localSize);
					return count;
				}
				tmpp = tmpp->next;
				count++;

			}

		}
		
	}
	printf("no if\n");
	struct func_start_end* tmp = (struct func_start_end*)malloc(sizeof(struct func_start_end));
	int label = userFuncs_stack->label;
	tmp = userFuncs_stack->next;
	userFuncs_stack = tmp;
	
	return label;
	
}
*/

unsigned userfuncs_searchfunc(SymbolTableEntry* sym){

	struct userfunc* tmp;
	tmp = userFuncs;
	int i = 0;

	assert(tmp && "Error: no functions to call...\n");

	for(i; i<totalUserFuncs; i++){
		if(strcmp( tmp->id, sym->name) == 0) return i;
		tmp = tmp->next;
	}

	assert(0 && "Error: Called fuction's name does not exist.\n");
	return 0;
}


// Prolly wrong.

unsigned userfuncs_newfunc (SymbolTableEntry* sym){
	if(call_flag == 1) return totalUserFuncs -1;
	if(func_start_end_flag == 0){
		if(totalUserFuncs == max_user_funcs){
			expand_consts_user();
		}
		struct userfunc* node;
		struct userfunc* tmp;
		int i=0;
		node = (struct userfunc*)malloc(sizeof(struct userfunc));
		node->id = sym->name;
		node->localSize = sym->totalLocals;
		node->address = sym->taddress;
		node->next = NULL;

		char* str1 = "_";


		if(totalUserFuncs == 0){
			userFuncs = node;

			struct func_start_end* tmp2 = (struct func_start_end*)malloc(sizeof(struct func_start_end));
			tmp2->label = totalUserFuncs;

			userFuncs_stack = tmp2;
		}
		else{
			tmp = userFuncs;

			while(tmp->next != NULL){
				tmp = tmp->next;
			}
			tmp->next = node;
	/*		struct userfunc* tmp = userFuncs;
			int u = 0;

			printf("---------------------------------i = %d, total = %d\n", i , totalUserFuncs);
			for(u; u<totalUserFuncs; u++){
				printf("Id = %s, addr = %d, size = %d\n", tmp->id, tmp->address, tmp->localSize);
				tmp = tmp->next;
			}
			u=0;*/

			struct func_start_end* tmp2 = (struct func_start_end*)malloc(sizeof(struct func_start_end));
			tmp2->label = totalUserFuncs;

			if(userFuncs_stack == NULL){
				userFuncs_stack = tmp2;
			}
			else{
				tmp2->next = userFuncs_stack;
				userFuncs_stack = tmp2;
			}
		}
		totalUserFuncs++;
		return totalUserFuncs-1;
	}
	else {
		struct func_start_end* tmp = (struct func_start_end*)malloc(sizeof(struct func_start_end));
		int label = userFuncs_stack->label;
		tmp = userFuncs_stack->next;
		userFuncs_stack = tmp;
		
		return label;
	}
}


void inst_expand(void){
	assert(inst_total == currInstruction);
	struct instruction* p = (struct instruction*) malloc(NEW_SIZE_INST);
	if(instructions){
		memcpy(p, instructions, CURR_SIZE_INST);
		free(instructions);
	}
	instructions = p;
	inst_total +=EXPAND_SIZE;
}

void inst_emit(struct instruction* t){
	if(currInstruction == inst_total){
		inst_expand();
	}


	struct instruction* p = instructions+currInstruction++;

	p->arg1 = t->arg1;
	p->arg2 = t->arg2;
	p->opcode = t->opcode;
	p->result = t->result;
	p->srcLine = t->srcLine;
	p = t;

	char* op = vmopcode_to_string(p->opcode);

	printf("=%d: %s ", p->srcLine, op);

	if(p->arg1) printf("arg1::%d - %d, ", p->arg1->type, p->arg1->val);
	if(p->arg2) printf("arg2::%d - %d, ", p->arg2->type, p->arg2->val);
	if(p->result) printf("result::%d - %d, ", p->result->type, p->result->val);

	printf("\n");

}

unsigned nextinstructionlabel(){
	return currInstruction;
}


static void avm_initstack (){
	unsigned i = 0;
	for(i; i<AVM_STACKSIZE; i++){
		AVM_WIPEOUT(stack[i]);
		stack[i].type = undef_m;
	}
}

void avm_tableincrefcounter (struct avm_table* t){
	++t->refCounter;
}

void avm_tablebucketsinit (struct avm_table_bucket** p){
	unsigned i = 0;
	for(i; i<AVM_TABLE_HASHSIZE; ++i){
		p[i] = (struct avm_table_bucket*) 0;
	}
}

struct avm_table* avm_tablenew () {
	struct avm_table* t = (struct avm_table*)malloc(sizeof(struct avm_table));
	AVM_WIPEOUT(*t);

	t->refCounter = t->total = 0;
	avm_tablebucketsinit(t->numIndexed);
	avm_tablebucketsinit(t->strIndexed);

	return t;
}

/*
void avm_memcellclear (struct avm_memcell* m){

	if(m->type != undef_m){
		memclear_func_t f = memclearFuncs[m->type];
		if(f) (*f)(m);
		m->type = undef_m;
	}

}

void avm_tablebucketsdestroy (struct avm_table_bucket** p){
	unsigned i = 0;
	struct avm_table_bucket* b;
	for(i; i<AVM_TABLE_HASHSIZE; ++i){
		b = *p;
		while(b){
			struct avm_table_bucket* del = b;
			b = b->next;
			avm_memcellclear(&del->key);
			avm_memcellclear(&del->value);
			free(del);
		}
		p[i] = (struct avm_table_bucket*) 0;
	}
}

void avm_tabledestroy (struct avm_table* t) {
	avm_tablebucketsdestroy(t->strIndexed);
	avm_tablebucketsdestroy(t->numIndexed);
	free(t);
}

*/
// Na baloume to intVal.
void make_operand (struct expr* e, struct vmarg* arg){

	

	switch(e->type){
		case var_e:
		case tableitem_e:
		case arithexpr_e:
		case boolexpr_e:
		case assignexpr_e:
		case newtable_e: {

			//assert(e->sym);
			arg->val = e->sym->offset;

			switch(e->sym->space){
				case programvar:{
					arg->type = global_a; break;
				}	
				case functionlocal: {arg->type = local_a;  break;}
				case formal_a:		{arg->type = formal_a; break;}
				default:			assert(0);
			}
			break;
		}

		case constbool_e: {
			if(e->boolConst == 'T') arg->val = 1;
			else  arg->val = 0;

			arg->type = bool_a; 
			break;
		}
		case conststring_e: {
			arg->val = consts_newstring(e->strConst);
			arg->type = string_a;
			break;
		}
		case constnum_e: {
			double k = (double)e->intConst;
			if(e->numConst != 0) arg->val = consts_newnumber(e->numConst);
			else arg->val = consts_newnumber((double)e->intConst);
			arg->type = number_a;
			break;
		}
		case nil_e: arg->type = nil_a; break;

		//Functions
		case programfunc_e: {
			arg->type = userfunc_a;
			arg->val = e->sym->iaddress;
			if(flag_call) arg->val = userfuncs_searchfunc(e->sym);
			else arg->val = userfuncs_newfunc(e->sym);
			break;
		}

		case libraryfunc_e: {
			arg->type = libfunc_a;
			arg->val = libfuncs_newused(e->sym->name);
			break;
		}
		default: 
			assert(0);

	}
}


void make_numberoperand (struct vmarg* arg, double val){
	arg->val = consts_newnumber(val);
	arg->type = number_a;
}

void make_booloperand (struct vmarg* arg, unsigned val){
	arg->val = val;
	arg->type = bool_a;
}

void make_retvaloperand (struct vmarg* arg){
	arg->type = retval_a;
}

void generate_inst(){
	unsigned i=0;
	for(i; i<currQuad; ++i){
		(*generators[quads[i].op]) (quads+i);
	}
}


void generate(enum vmopcode op, struct quad* quad){
	call_flag = 1;
	flag_call = 1;
	struct instruction* t = (struct instruction*) malloc(sizeof(struct instruction));
	t->opcode = op;
	t->srcLine = nextinstructionlabel();

	if(quad->arg1){
		t->arg1 =   (struct vmarg*)malloc(sizeof(struct vmarg));
		make_operand(quad->arg1, t->arg1);
	} 

	if(quad->arg2){
		t->arg2 =   (struct vmarg*)malloc(sizeof(struct vmarg));
		make_operand(quad->arg2, t->arg2);
	} 
	if(quad->result){
		
		t->result = (struct vmarg*)malloc(sizeof(struct vmarg));
		make_operand(quad->result, t->result);
	} 
	quad->taddress = nextinstructionlabel();
	inst_emit(t);
	call_flag = 0;
	flag_call = 0;
}

void generate_ADD (quad* quad) { generate(add_v, quad); }
void generate_SUB (quad* quad) { generate(sub_v, quad); }
void generate_MUL (quad* quad) { generate(mul_v, quad); }
void generate_DIV (quad* quad) { generate(div_v, quad); }
void generate_MOD (quad* quad) { generate(mod_v, quad); }
void generate_NEWTABLE 		(quad* quad) { generate(newtable_v, quad); }
void generate_TABLEGETELEM 	(quad* quad) { generate(tablegetelem_v, quad); }
void generate_TABLESETELEM 	(quad* quad) { generate(tablesetelem_v, quad); }
void generate_ASSIGN 		(quad* quad) { generate(assign_v, quad); }
void generate_NOP (quad* quad) { struct instruction* t; t->opcode=nop_v; inst_emit(t); }
void generate_relational (enum vmopcode op, quad* quad) {

	struct instruction* t = (struct instruction*) malloc(sizeof(struct instruction));
	t->srcLine = nextinstructionlabel();
	t->opcode = op;
	if(quad->arg1){
		t->arg1 =   (struct vmarg*)malloc(sizeof(struct vmarg));
		make_operand(quad->arg1, t->arg1);
	}
	if(quad->arg2){
		t->arg2 =   (struct vmarg*)malloc(sizeof(struct vmarg));
		make_operand(quad->arg2, t->arg2);
	} 
	t->result = (struct vmarg*)malloc(sizeof(struct vmarg));
	t->result->type = label_a;
	if(quad->label < currQuad){
		t->result->val = quads[quad->label].taddress;
	}
	else{
		add_incomplete_jump(nextinstructionlabel(), quad->label);
	}
	quad->taddress = nextinstructionlabel();
	inst_emit(t);

}
void generate_JUMP 			(quad* quad) 	{generate_relational(jump_v, quad);}
void generate_IF_EQ 		(quad* quad) 	{generate_relational(jeq_v, quad); }
void generate_IF_NOTEQ		(quad* quad) 	{generate_relational(jne_v, quad); }
void generate_GREATER	(quad* quad) 		{generate_relational(jgt_v, quad); }
void generate_GREATEREQ	(quad* quad) 		{generate_relational(jge_v, quad); }
void generate_LESS 		(quad* quad) 		{generate_relational(jlt_v, quad); }
void generate_LESSEQ 	(quad* quad) 		{generate_relational(jle_v, quad); }
void generate_NOT (quad* quad){
	quad->taddress = nextinstructionlabel();
	struct instruction* t  = (struct instruction*) malloc(sizeof(struct instruction));

	t->opcode = jeq_v;
	make_operand(quad->arg1, t->arg1);
	make_booloperand(t->arg2, 0);
	t->result->type = label_a;
	t->result->val = nextinstructionlabel() +3;
	inst_emit(t);

	t->opcode = assign_v;
	make_booloperand(t->arg1, 0);
	//reset_operand(t->arg2);
	make_operand(quad->result, t->result);
	inst_emit(t);
	t->opcode = jump_v;
	//reset_operand(t->arg1);
	//reset_operand(t->arg2);
	t->result->type = label_a;
	t->result->val = nextinstructionlabel() +2;
	inst_emit(t);

	t->opcode = assign_v;
	make_booloperand(t->arg1, 1);
	//reset_operand(t->arg2);
	make_operand(quad->result, t->result);
	inst_emit(t);
}
void generate_OR (quad* quad){
	quad->taddress = nextinstructionlabel();
	struct instruction* t  = (struct instruction*) malloc(sizeof(struct instruction));

	t->opcode = jeq_v;
	make_operand(quad->arg1, t->arg1);
	make_booloperand(t->arg2, 1);
	t->result->type = label_a;
	t->result->val = nextinstructionlabel()+4;
	inst_emit(t);

	make_operand(quad->arg2, t->arg1);
	t->result->val = nextinstructionlabel()+3;
	inst_emit(t);

	t->opcode = assign_v;
	make_booloperand(t->arg1, 0);
	//reset_operand(t->arg2);
	make_operand(quad->result, t->result);
	inst_emit(t);

	t->opcode = jump_v;
	//reset_operand(t->arg1);
	//reset_operand(t->arg2);
	t->result->type = label_a;
	t->result->val = nextinstructionlabel()+2;
	inst_emit(t);

	t->opcode = assign_v;
	make_booloperand(t->arg1, 1);
	//reset_operand(t->arg2);
	make_operand(quad->result, t->result);
	inst_emit(t);
}
void generate_AND (quad* quad){
	quad->taddress = nextinstructionlabel();
	struct instruction* t  = (struct instruction*) malloc(sizeof(struct instruction));

	t->opcode = jeq_v;
	make_operand(quad->arg1, t->arg1);
	make_booloperand(t->arg2, 1);
	t->result->type = label_a;
	t->result->val = nextinstructionlabel()+4;
	inst_emit(t);

	make_operand(quad->arg2, t->arg1);
	t->result->val = nextinstructionlabel()+3;
	inst_emit(t);

	t->opcode = assign_v;
	make_booloperand(t->arg1, 0);
	//reset_operand(t->arg2);
	make_operand(quad->result, t->result);
	inst_emit(t);

	t->opcode = jump_v;
	//reset_operand(t->arg1);
	//reset_operand(t->arg2);
	t->result->type = label_a;
	t->result->val = nextinstructionlabel()+2;
	inst_emit(t);

	t->opcode = assign_v;
	make_booloperand(t->arg1, 1);
	//reset_operand(t->arg2);
	make_operand(quad->result, t->result);
	inst_emit(t);
}
void generate_PARAM(quad* quad){
	quad->taddress = nextinstructionlabel();
	struct instruction* t  = (struct instruction*) malloc(sizeof(struct instruction));
	t->arg1 =   (struct vmarg*)malloc(sizeof(struct vmarg));
	t->opcode = pusharg_v;
	t->srcLine = nextinstructionlabel();
	make_operand(quad->arg1, t->arg1);
	inst_emit(t);
}
void generate_CALL(quad* quad){
	call_flag = 1;
	flag_call = 1;
	quad->taddress = nextinstructionlabel();
	struct instruction* t  = (struct instruction*) malloc(sizeof(struct instruction));
	t->arg1 =   (struct vmarg*)malloc(sizeof(struct vmarg));
	t->opcode = call_v;
	t->srcLine = nextinstructionlabel();
	make_operand(quad->arg1, t->arg1);
	inst_emit(t);
	call_flag = 0;
	flag_call = 0;
}

void generate_GETRETVAL(quad* quad){
	quad->taddress = nextinstructionlabel();
	struct instruction* t  = (struct instruction*) malloc(sizeof(struct instruction));
	t->arg2 = (struct vmarg*)malloc(sizeof(struct vmarg));
	t->opcode = assign_v;
	t->srcLine = nextinstructionlabel();
	make_operand(quad->result, t->arg2);
	t->arg1 = (struct vmarg*)malloc(sizeof(struct vmarg));
	make_retvaloperand(t->arg1);
	inst_emit(t);
}
void generate_FUNCSTART(quad* quad){
	func_start_end_flag = 0;
	SymbolTableEntry* f = quad->arg1->sym;
	f->taddress = nextinstructionlabel();

	push_funcstack(f);

	struct instruction* t  = (struct instruction*) malloc(sizeof(struct instruction));

	t->arg1 = (struct vmarg*)malloc(sizeof(struct vmarg));
	t->opcode = funcenter_v;
	t->srcLine = nextinstructionlabel();
	make_operand(quad->arg1, t->arg1);
	inst_emit(t);
}
void generate_RETURN(quad* quad){
	quad->taddress = nextinstructionlabel();
	struct instruction* t  = (struct instruction*) malloc(sizeof(struct instruction));
	t->srcLine = nextinstructionlabel();
	t->arg1 = (struct vmarg*)malloc(sizeof(struct vmarg));
	t->result = (struct vmarg*)malloc(sizeof(struct vmarg));
	t->opcode = assign_v;

	if(quad->arg1){
	make_retvaloperand(t->result);
	make_operand(quad->arg1, t->arg1);

	}
	
	inst_emit(t);

	struct func_stack* f = funcstack;
	append(f->returnList, nextinstructionlabel());
/*
	t->opcode = jump_v;
	//reset_operand(t->arg1);
	//reset_operand(t->arg2);
	t->srcLine = nextinstructionlabel();
	t->arg1->type = label_a;
	inst_emit(t);*/
}
void generate_FUNCEND(quad* quad){
	func_start_end_flag = 1;
	struct func_stack* f = pop_funcstack();
	quad->taddress = nextinstructionlabel();
	struct instruction* t = (struct instruction*) malloc(sizeof(struct instruction));
	t->arg1 = (struct vmarg*)malloc(sizeof(struct vmarg));
	t->srcLine = nextinstructionlabel();
	
	t->opcode = funcexit_v;

	make_operand(quad->arg1, t->arg1);
	inst_emit(t);
}

//----------------------PHASE 3 FUNCS-----------------------------------------------------------------------------

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
	int i;
	struct expr* result;
	int count;


	if(reversed_elist == NULL) count = 0;
	else count = reversed_elist->countParams;

	struct expr* func = emit_iftableitem(lv, currQuad, line);
	func->sym->space = currscopespace();
	func->sym->offset = currscopeoffset();
	//incurrscopeoffset();
	for(i=0;i<count;i++){
		emit(param , reversed_elist , NULL, NULL, currQuad, line);
		reversed_elist = reversed_elist-> next;
		PopElist();
	}
	/*while(reversed_elist){
		printf("in make call while %s\n");
		emit(param , reversed_elist , NULL, NULL, currQuad, line);
		reversed_elist = reversed_elist-> next;
	}*/

	emit(call, func, NULL , NULL, currQuad, line);
	result = newexpr (var_e);
	result->sym = newtemp();
	result->sym->space = currscopespace();
	result->sym->offset = currscopeoffset();
	incurrscopeoffset();
	emit(getretval , NULL, NULL, result, currQuad, line);

	return result;
}

struct expr* newexpr_conststring (char* s){
	struct expr* e = newexpr(conststring_e);
	e->sym = NULL;
	e->strConst = strdup(s);
	return e;
}

struct expr* newexpr_constnum (double n){
	struct expr* e = newexpr(constnum_e);
	e->sym = NULL;
	e->numConst = n;
	return e;
}

struct expr* newexpr_constbool (unsigned char b){
	struct expr* e = newexpr(constbool_e);
	e->sym = NULL;
	e->boolConst = b;
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
	if(l1->label == -1){
		return l2;
	}
	else if (l2->label == -1){
		return l1;
	}
	else{
		struct break_cont* tmp = l1;
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = l2;
		return l1;
	}
}

struct expr* emit_iftableitem(struct expr* e, unsigned label, unsigned line){
	if(e->type != tableitem_e){
		return e;
	}
		struct expr* result = newexpr(var_e);
		result->sym = newtemp();
		result->sym->space = currscopespace();
		result->sym->offset = currscopeoffset();
		incurrscopeoffset();
		emit(tablegetelem, e, e->index, result, label, line);
		if(result->sym == NULL) printf("result = null'n");
		return result;
}

struct expr* member_item (struct expr* lv, char* name, unsigned label, unsigned line){
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
	struct expr* tmp = elists;
	 elists = elists->next;
	if(elists == NULL) paramCount = 0;
	else paramCount = elists->countParams;


}

void patchlist (struct break_cont* list, int label){
	struct break_cont*tmp = list;
	if(tmp->label == -1) return;
	while(tmp){
		quads[tmp->label].result = (struct expr*) malloc(sizeof(struct expr));
		quads[tmp->label].result->intConst = label;
		quads[tmp->label].taddress = label;
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
		expand();
	}
	char* sop = (char*)malloc(sizeof(char)*15);
	quad* p = quads+currQuad++;
	p->op = op;
	p->arg1 = arg1;
	p->arg2 = arg2;
	p->result = result;
	p->label = label;
	p->line = line;

	sop = iopcode_to_string(op);

/*	if(strcmp(sop,"jump") == 0){
		p->taddress = arg2->intConst;
	}
	else if(strcmp(sop,"if_lesseq")==0 || strcmp(sop,"if_greatereq") == 0 || strcmp(sop,"if_less") ==0 || strcmp(sop,"if_greater") ==0 || strcmp(sop,"if_eq") ==0 || strcmp(sop,"if_noteq") ==0){
		p->taddress = result->intConst;
	}*/




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

char* vmopcode_to_string (enum vmopcode op){

	char* sop = (char*)malloc(sizeof(char)*15);
	if(op == assign_v){
			strcpy(sop, "assign");
		}else if(op == add_v){
			strcpy(sop, "add");
		}else if(op == sub_v){
			strcpy(sop, "sub");
		}else if(op == mul_v){
			strcpy(sop, "mul");
		}else if(op == div_v) {
			strcpy(sop, "div");
		}else if(op == mod_v) {
			strcpy(sop, "mod");
		}else if(op == uminus_v) {
			strcpy(sop, "uminus");
		}else if(op == and_v) {
			strcpy(sop, "and");
		}else if(op == or_v) {
			strcpy(sop, "or");
		}else if(op == not_v) {
			strcpy(sop, "not");
		}else if(op == jeq_v) {
			strcpy(sop, "jeq");
		}else if(op == jne_v) {
			strcpy(sop, "jne");
		}else if(op == jle_v) {
			strcpy(sop, "jle");
		}else if(op == jge_v) {
			strcpy(sop, "jge");
		}else if(op == jlt_v) {
			strcpy(sop, "jlt");
		}else if(op == jgt_v) {
			strcpy(sop, "jgt");
		}else if(op == call_v) {
			strcpy(sop, "call");
		}else if(op == pusharg_v) {
			strcpy(sop, "pusharg");
		}else if(op == funcenter_v) {
			strcpy(sop, "funcenter");
		}else if(op == funcexit_v) {
			strcpy(sop, "funcexit");
		}else if(op == tablegetelem_v) {
			strcpy(sop, "tablegetelem");
		}else if(op == tablesetelem_v) {
			strcpy(sop, "tablesetelem");
		}else if(op == newtable_v) {
			strcpy(sop, "newtable");
		}else if(op == newtable_v) {
			strcpy(sop, "newtable");
		}else if(op == jump_v){
			strcpy(sop, "jump");
		}else if(op == nop_v){
			strcpy(sop, "nop");
		}
		return sop;
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

			if(quads[i].arg1->sym) 					printf("%s %s ", op, quads[i].arg1->sym->name);	
			else if(quads[i].arg1->intConst) 		printf("%s %d ", op, quads[i].arg1->intConst);
			else if(quads[i].arg1->numConst != 0) 		printf("num %s %f ", op, quads[i].arg1->numConst);
			else if(quads[i].arg1->boolConst == 'T' && quads[i].arg1->boolConst == 'F' ) 		printf("bool %s %c ", op, quads[i].arg1->boolConst);
			else 									printf("%s %s ", op, quads[i].arg1->strConst);

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

			if (quads[i].result->sym) 						printf("%s [line %d]\n", quads[i].result->sym->name, quads[i].line);
			else if(quads[i].result->numConst != 0)  		printf("%f [line %d]\n", quads[i].result->numConst, quads[i].line);
			else if (quads[i].result->strConst != NULL) 	printf("%s [line %d]\n", quads[i].result->strConst, quads[i].line);
			else 											printf("%d [line %d]\n", quads[i].result->intConst, quads[i].line);

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
			

			if(quads[i].arg1->sym) 																fprintf(fp, "%s %s ", op, quads[i].arg1->sym->name);	
			else if(quads[i].arg1->intConst) 													fprintf(fp, "%s %d ", op, quads[i].arg1->intConst);
			else if(quads[i].arg1->numConst != 0) 												fprintf(fp, "%s %f ", op, quads[i].arg1->numConst);
			else if(quads[i].arg1->boolConst == 'T' && quads[i].arg1->boolConst == 'F' ) 		fprintf(fp, "%s %c ", op, quads[i].arg1->boolConst);
			else 																				fprintf(fp, "%s %s ", op, quads[i].arg1->strConst);
			

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
	node->space = currscopespace();
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
			return -1;
		}
		
	}
	


	if(scope != 0){
		while(i<=scope){
			if(tmp->next == NULL) {
				return -1;
			}
			tmp = tmp->next;
			i++;
		}
	}

	if(tmp->scopeHead == NULL){
		return -1;
	}
	tmp2 = tmp->scopeHead;

	while(tmp2!=NULL){
		if(strcmp(tmp2->name, name) == 0 && tmp2->isActive == 1){
			return tmp2->offset;
		}
		tmp2=tmp2->scopeNext;
	}

	return -1;
}

/* 1 if exists
else 0*/
int Lookup(char *ttype, char *name, unsigned int scope){
	int i;

	for(i=scope;i>=0;i--){
		if(LookupInScopeVar(i, name, ttype)!=-1){
			return LookupInScopeVar(i, name, ttype);
		}
	}
	return -1;
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

int LookUpinScopeFormal (unsigned int scope, char* name){

	int i = 0;

	ScopesHeadsList *tmp = scopesHeadsList;
	SymbolTableEntry *tmp2;
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
		if(strcmp(tmp2->name, name) == 0 && tmp2->isActive == 1 && tmp2->space == 2){
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
	for(i=0;i<=scope;i++){
		
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
		if(tmp2->type==LOCAL || tmp2->type==FORMAL || tmp2->type== USERFUNC){
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









