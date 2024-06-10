%{
	#include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #define YY_DECL int alpha_yylex (void* yylval)
  
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
LOCAL "local"
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
DOT \.
DOUBLE_DOT \..

ID [a-zA-Z][a-zA-Z_0-9]*
SPACE [\r \t\v]
NEW_LINE "\n"
COMMENT1 "//".*
COMMENT2 "/*"
OTHER .

%option noyywrap
%option yylineno
%option nounistd
	
%%
{IF}  {addToList(yylineno, numtoken++, "KEYWORD", "if","IF", "<-enumerated");}
{ELSE}  {addToList(yylineno, numtoken++, "KEYWORD","else", "ELSE", "<-enumerated");}
{WHILE} {addToList(yylineno, numtoken++, "KEYWORD","while", "WHILE", "<-enumerated");}
{FOR}   {addToList(yylineno, numtoken++, "KEYWORD","for", "FOR", "<-enumerated");}
{FUNCTION}  {addToList(yylineno, numtoken++, "KEYWORD","function", "FUNCTION", "<-enumerated");}
{RETURN}  {addToList(yylineno, numtoken++, "KEYWORD","return", "RETURN", "<-enumerated");}
{BREAK} {addToList(yylineno, numtoken++, "KEYWORD","break", "BREAK", "<-enumerated");}
{CONTINUE}  {addToList(yylineno, numtoken++, "KEYWORD", "continue", "CONTINUE", "<-enumerated");}
{AND} {addToList(yylineno, numtoken++, "KEYWORD","and",  "AND","<-enumerated");}
{NOT} {addToList(yylineno, numtoken++, "KEYWORD","not", "NOT", "<-enumerated");}
{OR}  {addToList(yylineno, numtoken++, "KEYWORD","or", "OR", "<-enumerated");}
{LOCAL} {addToList(yylineno, numtoken++, "KEYWORD","local", "LOCAL", "<-enumerated");}
{TRUE}  {addToList(yylineno, numtoken++, "KEYWORD","true", "TRUE", "<-enumerated");}
{FALSE} {addToList(yylineno, numtoken++, "KEYWORD","false", "FALSE", "<-enumerated");}
{NIL} {addToList(yylineno, numtoken++, "KEYWORD","nil", "NIL", "<-enumerated");}

{INSERT}  {addToList(yylineno, numtoken++, "OPERATOR","=","INSERT", "<-enumerated");}
{PLUS}  {addToList(yylineno, numtoken++, "OPERATOR","+","PLUS", "<-enumerated");}
{MINUS} {addToList(yylineno, numtoken++, "OPERATOR","-","MINUS", "<-enumerated");}
{MULT}  {addToList(yylineno, numtoken++, "OPERATOR","*","MULT", "<-enumerated");}
{DIV} {addToList(yylineno, numtoken++, "OPERATOR","/","DIV", "<-enumerated");}
{MOD} {addToList(yylineno, numtoken++, "OPERATOR","%","MOD", "<-enumerated");}
{EQUAL} {addToList(yylineno, numtoken++, "OPERATOR","==","EQUAL", "<-enumerated");}
{NOT_EQUAL} {addToList(yylineno, numtoken++, "OPERATOR","!=","NOT_EQUAL", "<-enumerated");}
{PLUS_ONE}  {addToList(yylineno, numtoken++, "OPERATOR","++","PLUS_ONE", "<-enumerated");}
{MINUS_ONE} {addToList(yylineno, numtoken++, "OPERATOR","--","MINUS_ONE", "<-enumerated");}
{GREATER} {addToList(yylineno, numtoken++, "OPERATOR",">","GREATER", "<-enumerated");}
{LESS}  {addToList(yylineno, numtoken++, "OPERATOR","<","LESS", "<-enumerated");}
{GREATER_EQUAL} {addToList(yylineno, numtoken++, "OPERATOR",">=", "GREATER_EQUAL","<-enumerated");}
{LESS_EQUAL}  {addToList(yylineno, numtoken++, "OPERATOR","<=","LESS_EQUAL", "<-enumerated");}

{INT} {	addToList(yylineno, numtoken++, "CONST_INT", yytext, "int", "<-integer");

	  }
{REAL}  {addToList(yylineno, numtoken++, "REAL_NUM", yytext, "REAL_NUMBER", "<-real_num");}
{STRING}  {
  int c;
  int flag = 1;
  int i = 0;
  char *tmp = (char*) malloc(sizeof(char));
  while((c=input()) != EOF){

      if(c == '\\'){
          if((c = input())!=EOF){
          switch (c){
            case 'n': *(tmp+i) = '\n'; printf("Warning string have \\n escape char\n"); break;
            case 't': *(tmp+i) = '\t'; printf("Warning string have \\t escape char\n"); break;
            case '\\': *(tmp+i) = '\\'; printf("Warning string have \\\ escape char\n"); break;
			case 'm' : *(tmp+i) = '\m'; printf("Warning string have \\m escape char\n"); break;
			case '\"' : *(tmp+i) = '\"'; printf("Warning string have \\m escape char\n"); break;
			case 'b':*(tmp+i) = 'b'; printf("Warning string have \\b escape char\n"); break;
			
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
  }
}


{L_BRACKET} {addToList(yylineno, numtoken++, "PUNCTUATION", "{","L_BRACKET", "<-enumerated");}
{R_BRACKET} {addToList(yylineno, numtoken++, "PUNCTUATION", "}","R_BRACKET", "<-enumerated");}
{L_SBRACKET}  {addToList(yylineno, numtoken++, "PUNCTUATION", "[","L_SBRACKET", "<-enumerated");}
{R_SBRACKET}  {addToList(yylineno, numtoken++, "PUNCTUATION", "]","R_SBRACKET", "<-enumerated");}
{L_PAREN} {addToList(yylineno, numtoken++, "PUNCTUATION", "(","L_PAREN", "<-enumerated");}
{R_PARENR}  {addToList(yylineno, numtoken++, "PUNCTUATION", ")","R_PARENR", "<-enumerated");}
{SEMICOL} {addToList(yylineno, numtoken++, "PUNCTUATION", ";","SEMICOL", "<-enumerated");}
{COMMA} {addToList(yylineno, numtoken++, "PUNCTUATION", ",","COMMA", "<-enumerated");}
{COL} {addToList(yylineno, numtoken++, "PUNCTUATION", ":","COL", "<-enumerated");}
{DOUBLE_COL}  {addToList(yylineno, numtoken++, "PUNCTUATION", "::","DOUBLE_COL", "<-enumerated");}
{DOT} {addToList(yylineno, numtoken++, "PUNCTUATION", ".","DOT", "<-enumerated");}
{DOUBLE_DOT}  {addToList(yylineno, numtoken++, "PUNCTUATION", "..","DOUBLE_DOT", "<-enumerated");}

{ID}  {addToList(yylineno, numtoken++, "ID", yytext, yytext, "<-char*");}
{NEW_LINE}			{}
{SPACE} {}
{OTHER}	{addToList(yylineno, numtoken++, "ERROR", yytext,"NOT_VALID_ELEMENT", "<-UNDEFINED");} 

{COMMENT1}  {addToList(yylineno, numtoken++, "COMMENT", "//","LINE_COMMENT", "<-enumerated");}
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
      }else{
	  	unput(c);
	  }
    }
    if(nest == 0){
		str = (char*)malloc(sizeof(char)*5);
		sprintf(str, "%d - %d", start, yylineno);

      addToList(start, numtoken++, "COMMENT", str,"MULTILINE_COMMENT", "<-enumerated");
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

int main(int argc, char** argv){
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
}
		   
