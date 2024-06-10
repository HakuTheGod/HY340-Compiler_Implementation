%{
	#include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include "parser.h"
  #define YY_DECL int alpha_yylex (void* val)
	
  
  void addToList(int line_no, int numtoken, char* type, char* val, char* name, char* type2);
  void Print_Token_list();
  
  struct alpha_token_t{
    int line_no;
    int numtoken;
    char* type;
    char* val;
    char* name;
    char* type2;
    struct alpha_token_t* next;
  };
  int numtoken = 1;
  struct alpha_token_t* head;
%}




IF "if"
ELSE "else"
WHILE "while"
FOR "for"
FUNCTION "function"
RETURN "return"
BREAK "break"
CONTINUE "continue"
AND "and"
NOT "not"
OR "or"
LOCALVAR "local"
TRUE "true"
FALSE "false"
NIL "nil"

INSERT "="
PLUS "+"
MINUS "-"
MULT "*"
DIV "/"
MOD "%"
EQUAL "=="
NOT_EQUAL "!="
PLUS_ONE "++"
MINUS_ONE "--"
GREATER ">"
LESS "<"
GREATER_EQUAL ">="
LESS_EQUAL "<="

INT (0[Xx][0-9A-Fa-f]+)|([0-9]*)
REAL [0-9]*\.[0-9](e|E)?[0-9]*
STRING \"

L_BRACKET \{
R_BRACKET \}
L_SBRACKET \[
R_SBRACKET \]
L_PAREN \(
R_PARENR \)
SEMICOL \;
COMMA \,
COL \:
DOUBLE_COL \::
DOT \.{1}
DOUBLE_DOT \.{2}

ID [a-zA-Z][a-zA-Z_0-9]*
SPACE [\r \t\v]
NEW_LINE "\n"
COMMENT1 "//".*
COMMENT2 "/*"
OTHER .

%option noyywrap
%option yylineno

	
%%
{IF}  {addToList(yylineno, numtoken++, "KEYWORD", "if","IF", "<-enumerated"); return IF;}
{ELSE}  {addToList(yylineno, numtoken++, "KEYWORD","else", "ELSE", "<-enumerated"); return ELSE;}
{WHILE} {addToList(yylineno, numtoken++, "KEYWORD","while", "WHILE", "<-enumerated"); return WHILE;}
{FOR}   {addToList(yylineno, numtoken++, "KEYWORD","for", "FOR", "<-enumerated"); return FOR;}
{FUNCTION}  {addToList(yylineno, numtoken++, "KEYWORD","function", "FUNCTION", "<-enumerated"); return FUNCTION;}
{RETURN}  {addToList(yylineno, numtoken++, "KEYWORD","return", "RETURN", "<-enumerated"); return RETURN;}
{BREAK} {addToList(yylineno, numtoken++, "KEYWORD","break", "BREAK", "<-enumerated"); return BREAK;}
{CONTINUE}  {addToList(yylineno, numtoken++, "KEYWORD", "continue", "CONTINUE", "<-enumerated"); return CONTINUE;}
{AND} {addToList(yylineno, numtoken++, "KEYWORD","and",  "AND","<-enumerated"); return AND;}
{NOT} {addToList(yylineno, numtoken++, "KEYWORD","not", "NOT", "<-enumerated"); return NOT;}
{OR}  {addToList(yylineno, numtoken++, "KEYWORD","or", "OR", "<-enumerated"); return OR;}
{LOCALVAR} {addToList(yylineno, numtoken++, "KEYWORD","local", "LOCAL", "<-enumerated"); return LOCALVAR;}
{TRUE}  {addToList(yylineno, numtoken++, "KEYWORD","true", "TRUE", "<-enumerated"); return TRUE;}
{FALSE} {addToList(yylineno, numtoken++, "KEYWORD","false", "FALSE", "<-enumerated"); return FALSE;}
{NIL} {addToList(yylineno, numtoken++, "KEYWORD","nil", "NIL", "<-enumerated"); return NIL;}

{INSERT}  {addToList(yylineno, numtoken++, "OPERATOR","=","INSERT", "<-enumerated"); return INSERT;}
{PLUS}  {addToList(yylineno, numtoken++, "OPERATOR","+","PLUS", "<-enumerated"); return PLUS;}
{MINUS} {addToList(yylineno, numtoken++, "OPERATOR","-","MINUS", "<-enumerated"); return MINUS;}
{MULT}  {addToList(yylineno, numtoken++, "OPERATOR","*","MULT", "<-enumerated"); return MULT;}
{DIV} {addToList(yylineno, numtoken++, "OPERATOR","/","DIV", "<-enumerated"); return DIV;}
{MOD} {addToList(yylineno, numtoken++, "OPERATOR","%","MOD", "<-enumerated"); return MOD;}
{EQUAL} {addToList(yylineno, numtoken++, "OPERATOR","==","EQUAL", "<-enumerated"); return EQUAL;}
{NOT_EQUAL} {addToList(yylineno, numtoken++, "OPERATOR","!=","NOT_EQUAL", "<-enumerated"); return NOT_EQUAL;}
{PLUS_ONE}  {addToList(yylineno, numtoken++, "OPERATOR","++","PLUS_ONE", "<-enumerated"); return PLUS_ONE;}
{MINUS_ONE} {addToList(yylineno, numtoken++, "OPERATOR","--","MINUS_ONE", "<-enumerated"); return MINUS_ONE;}
{GREATER} {addToList(yylineno, numtoken++, "OPERATOR",">","GREATER", "<-enumerated"); return GREATER;}
{LESS}  {addToList(yylineno, numtoken++, "OPERATOR","<","LESS", "<-enumerated"); return LESS;}
{GREATER_EQUAL} {addToList(yylineno, numtoken++, "OPERATOR",">=", "GREATER_EQUAL","<-enumerated"); return GREATER_EQUAL;}
{LESS_EQUAL}  {addToList(yylineno, numtoken++, "OPERATOR","<=","LESS_EQUAL", "<-enumerated"); return LESS_EQUAL;}

{INT} {	addToList(yylineno, numtoken++, "CONST_INT", yytext, "int", "<-integer"); yylval.intVal=atoi(yytext); return INT;

	  }
{REAL}  {addToList(yylineno, numtoken++, "REAL_NUM", yytext, "REAL_NUMBER", "<-real_num"); yylval.realVal=atoi(yytext); return REAL;}
{STRING}  {
  int c;
  int flag = 1;
  int i = 0;
  char *tmp = (char*) malloc(sizeof(char));
  while((c=input()) != EOF){

      if(c == '\\'){
          if((c = input())!=EOF){
          switch (c){
              case 'n': *(tmp+i) = '\n'; break;
              case 't': *(tmp+i) = '\t'; break;
              case '\\': *(tmp+i) = '\\'; break;
			  case 'b':*(char*)(tmp+i) = '\\b'; printf("Warning string have \\b escape char\n"); break; 
          }
          i++;
		}else

			break;
      }
      else if(c == '\"'){
          flag=0;
          
          break;
      }
      else{

          *(tmp+i)=c;
		  tmp=(char*)realloc(tmp , ((i++)+1) * sizeof(char));
      	*(tmp+i)= '\0';

      }
      
  }
  numtoken++;
  if(flag == 1){
      printf("error wait for \" to close the string in line %d\n", yylineno);
  }
  else{
      addToList(yylineno, numtoken, "STRING", tmp, tmp, "<-char*");

      yylval.strVal = strdup(yytext);

	  return STRING;
  }
}


{L_BRACKET} {addToList(yylineno, numtoken++, "PUNCTUATION", "{","L_BRACKET", "<-enumerated"); return L_BRACKET;}
{R_BRACKET} {addToList(yylineno, numtoken++, "PUNCTUATION", "}","R_BRACKET", "<-enumerated"); return R_BRACKET;}
{L_SBRACKET}  {addToList(yylineno, numtoken++, "PUNCTUATION", "[","L_SBRACKET", "<-enumerated"); return L_SBRACKET;}
{R_SBRACKET}  {addToList(yylineno, numtoken++, "PUNCTUATION", "]","R_SBRACKET", "<-enumerated"); return R_SBRACKET;}
{L_PAREN} {addToList(yylineno, numtoken++, "PUNCTUATION", "(","L_PAREN", "<-enumerated"); return L_PAREN;}
{R_PARENR}  {addToList(yylineno, numtoken++, "PUNCTUATION", ")","R_PARENR", "<-enumerated"); return R_PARENR;}
{SEMICOL} {addToList(yylineno, numtoken++, "PUNCTUATION", ";","SEMICOL", "<-enumerated"); return SEMICOL;}
{COMMA} {addToList(yylineno, numtoken++, "PUNCTUATION", ",","COMMA", "<-enumerated"); return COMMA;}
{COL} {addToList(yylineno, numtoken++, "PUNCTUATION", ":","COL", "<-enumerated"); return COL;}
{DOUBLE_COL}  {addToList(yylineno, numtoken++, "PUNCTUATION", "::","DOUBLE_COL", "<-enumerated"); return DOUBLE_COL;}
{DOT} {addToList(yylineno, numtoken++, "PUNCTUATION", ".","DOT", "<-enumerated"); return DOT;}
{DOUBLE_DOT}  {addToList(yylineno, numtoken++, "PUNCTUATION", "..","DOUBLE_DOT", "<-enumerated"); return DOUBLE_DOT;}

{ID}  {addToList(yylineno, numtoken++, "ID", yytext, yytext, "<-char*"); yylval.strVal = strdup(yytext); return ID;}
{NEW_LINE}			{}
{SPACE} {}
{OTHER}	{addToList(yylineno, numtoken++, "ERROR", yytext,"NOT_VALID_ELEMENT", "<-UNDEFINED"); return OTHER;} 

{COMMENT1}  {addToList(yylineno, numtoken++, "COMMENT", "//","LINE_COMMENT", "<-enumerated"); return COMMENT1;}
{COMMENT2}  {
  int nest = 1;
	int start = yylineno;
	int	cur = yylineno;
	char* line;
	char* str;
  char c;
  while((c=input()) != EOF){
    if(c == '*'){
      c = input();
      if(c == '/'){
        nest--;
		  if(nest>=1){
			line = (char*)malloc(sizeof(char));  
			sprintf(line,"%d", yylineno);
		  	addToList(yylineno, numtoken++, "COMMENT", line,"NESTED_COMMENT", "<-enumerated");
		  }
      }

    }else if(c == '/'){
      c = input();
      if(c == '*'){
		  
        nest++;
      }
    }
    if(nest == 0){
		str = (char*)malloc(sizeof(char)*5);
		sprintf(str, "%d - %d", start, yylineno);

      addToList(start, numtoken++, "COMMENT", str,"MULTILINE_COMMENT", "<-enumerated");
		return COMMENT2;
      break;
    }
  }
  if(nest!=0){
    printf("error wait for '*''/' to close the string in line %d\n", yylineno);
  }
}
%%


void addToList(int line_no, int numtoken, char* type, char* val, char* name, char* type2){
  struct alpha_token_t *node,*prev;
  node = (struct alpha_token_t*)malloc(sizeof(struct alpha_token_t));

  node->line_no = line_no;
  node->numtoken = numtoken;
  node->type = type;
  node->val = strdup(val);
  node->name = strdup(name);
  node->type2 = type2;
  node->next = NULL;

  if(head == NULL){
	  head =node;
	  
    return;
  }
	if(head->next == NULL){
		head->next = node;
		return;
	}
	prev = head;

  while(prev->next!=NULL){
    prev = prev->next;
  }
  prev->next = node;
  return;
}


void Print_Token_list(){
  struct alpha_token_t *node;
  node = head;
  if(node == NULL)
  	printf("list is empty\n");
  while (node != NULL) {

	  printf("%3d: #%3d [%s] \t\t %s %s %s\n",node->line_no,node->numtoken,node->val,node->type, node->name, node->type2);
    node = node->next;
  }
}

/*int main(int argc, char** argv){
  if(argc > 1){

    if(!(yyin = fopen(argv[1], "r"))){

      	fprintf(stderr, "Cannot read file %s\n", argv[1]);
      return 1;
    }
  }
  else {
    yyin = stdin;
  }
	alpha_yylex(NULL);

  Print_Token_list();
	fclose(yyin);
  return 0;
}*/
		   
