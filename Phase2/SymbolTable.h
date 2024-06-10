#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HASH_MULTIPLIER 67458
#define MAX_SIZE 1021


enum SymbolType {
	GLOBAL, LOCAL, FORMAL,
	USERFUNC, LIBFUNC
};

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
	int isActive;
	const char *name;
	FuncArgs *head;
	unsigned int scope;
	unsigned int line;
	enum SymbolType type;
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

ScopesHeadsList *scopesHeadsList;


int legthOfScopesHeadsList = 0;

void InsertVar(SymbolTable *hashTable,int key,char *name, unsigned int scope, unsigned int line, int type);
void InsertFunc(SymbolTable *hashTable,int key,char *name, unsigned int scope, unsigned int line, int type, FuncArgs *funcargs);
int LookupInScopeVar(unsigned int scope, char *name, char *type);
void Hide(unsigned int scope);
void PrintScopes();
void PrintHash (SymbolTable *hashTable);
int LookUpinScopeFunc (unsigned int scope, char *name, char *type);


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

void InsertVar(SymbolTable *hashTable,int key,char *name, unsigned int scope, unsigned int line, int type){

	SymbolTableEntry *node, *tmp2;
	ScopesHeadsList *tmp;

	ScopesHeadsList *nxt = (ScopesHeadsList*)malloc(sizeof(ScopesHeadsList));

	int i;
	unsigned int hash = SymbolTable_hash(key)%MAX_SIZE;
	
	node = (SymbolTableEntry*) malloc(sizeof(SymbolTableEntry));
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
		
		return;
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
		return;
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
	
}


void InsertFunc(SymbolTable *hashTable,int key,char *name, unsigned int scope, unsigned int line, int type, FuncArgs *funcargs){

	
	SymbolTableEntry *node, *tmp2;
	ScopesHeadsList *tmp;

	ScopesHeadsList *nxt = (ScopesHeadsList*)malloc(sizeof(ScopesHeadsList));

	int i;
	unsigned int hash = SymbolTable_hash(key)%MAX_SIZE;
	
	node = (SymbolTableEntry*) malloc(sizeof(SymbolTableEntry));
	node->isActive = 1;
	node->name = name;
	node->scope = scope;
	node->line = line;
		switch (type)
	{
	case 1:
		node->type = LIBFUNC;
		break;
	default:
		node->type = USERFUNC;
		break;
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
		
		return;
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
		return;
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

/* 1 if exists
else 0*/
int LookupFunc(char *ttype, char *name, unsigned int scope){

}



void Hide(unsigned int scope){


	int i;
	ScopesHeadsList *tmp = scopesHeadsList;
	SymbolTableEntry *tmp2;

	for(i=0;i<scope;i++){
		tmp = tmp->next;
	}

	tmp2 = tmp->scopeHead;

	if(tmp2 == NULL) return;

	while(tmp2!=NULL){

		tmp2->isActive = 0;
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
			switch(tmp2->type){
				case 0: strcpy(type, "Global variable");
				case 1: strcpy(type, "Local variable");
				case 2: strcpy(type, "Formal variable");
				case 3: strcpy(type, "User function");
				default: strcpy(type, "Library function");
			}

			printf("  \"%s\"   [%s]  (line %d) (scope %d) isActive: %d\n",tmp2->name, type,  tmp2->line, tmp2->scope, tmp2->isActive);
			tmp2=tmp2->scopeNext;
		}
		printf("  \"%s\"   [%s]  (line %d) (scope %d) isActive: %d\n",tmp2->name, type,  tmp2->line, tmp2->scope, tmp2->isActive);
		
		}
		tmp= tmp->next;
	}


	if(tmp->scopeHead != NULL){
			printf("---------------  Scope: #%d ---------------\n", tmp->scopeHead->scope);
		tmp2 = tmp->scopeHead;
		while (tmp2->scopeNext != NULL)
		{
			printf("  \"%s\"   [%s]  (line %d) (scope %d) isActive: %d\n",tmp2->name, type, tmp2->line, tmp2->scope, tmp2->isActive);
			tmp2=tmp2->scopeNext;
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

