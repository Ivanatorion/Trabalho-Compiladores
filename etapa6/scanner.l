%{
#include <stdio.h>
#include <string.h>
#include "parser.tab.h"
#include "include/defines.h"

#define YYSTYPE valLex

int _LINE_NUMBER_UNIQUE_VAR_NAME = 1;
int _IS_LINE_COMMENT = 0;
int _IS_OTHER_COMMENT = 0;

#define RET_TOK(TOKEN) if(!_IS_LINE_COMMENT && !_IS_OTHER_COMMENT) return TOKEN;

int get_line_number();

void setYyVal(int tipo_tok, int tipo_lit){
  if(_IS_LINE_COMMENT || _IS_OTHER_COMMENT)
    return;

  yylval.valor_lexico.line_number = get_line_number();
  yylval.valor_lexico.tipo_token = tipo_tok;
  yylval.valor_lexico.tipo_literal = tipo_lit;

  switch(tipo_lit){
    case TL_NONE:
      if(tipo_tok != TT_CE && tipo_tok != TT_PR)
        yylval.valor_lexico.valTokStr = strdup(yytext);
      else
        yylval.valor_lexico.valTokStr = NULL;
      break;
    case TL_INT:
      yylval.valor_lexico.valTokInt = atoi(yytext);
      break;
    case TL_BOOL:
      yylval.valor_lexico.valTokBool = (strcmp(yytext, "false") ? 1 : 0);
      break;
    case TL_FLOAT:
      yylval.valor_lexico.valTokFloat = atof(yytext);
      break;
    case TL_CHAR:
      yylval.valor_lexico.valTokChar = yytext[1];
      break;
    case TL_STRING:
      yytext[strlen(yytext) - 1] = '\0';
      yylval.valor_lexico.valTokStr = strdup(yytext+1);
      break;
  }
}

%}

especChar [\[,\],\,\;\:\(\)\{\}\+\-\|\?\@\*\/\<\>\=\!\&\%\#\^\.\$\~\`]
digit [0-9]
litInt {digit}+
cExp [eE][-+]?{litInt}?
litFloat {litInt}?("."{litInt}){cExp}?

identificador [a-zA-Z_][a-zA-Z0-9_]*

%%

"int" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_INT); }
"float" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_FLOAT); }
"bool" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_BOOL); }
"char" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_CHAR); }
"string" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_STRING); }
"if" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_IF); }
"then" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_THEN); }
"else" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_ELSE); }
"while" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_WHILE); }
"do" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_DO); }
"input" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_INPUT); }
"output" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_OUTPUT); }
"return" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_RETURN); }
"const" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_CONST); }
"static" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_STATIC); }
"foreach" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_FOREACH); }
"for" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_FOR); }
"switch" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_SWITCH); }
"case" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_CASE); }
"break" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_BREAK); }
"continue" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_CONTINUE); }
"class" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_CLASS); }
"private" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_PRIVATE); }
"public" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_PUBLIC); }
"protected" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_PROTECTED); }
"end" { setYyVal(TT_PR, TL_NONE); RET_TOK(TK_PR_END); }

"<=" { setYyVal(TT_OC, TL_NONE); RET_TOK( TK_OC_LE ); }
">=" { setYyVal(TT_OC, TL_NONE); RET_TOK( TK_OC_GE ); }
"==" { setYyVal(TT_OC, TL_NONE); RET_TOK( TK_OC_EQ ); }
"!=" { setYyVal(TT_OC, TL_NONE); RET_TOK( TK_OC_NE ); }
"&&" { setYyVal(TT_OC, TL_NONE); RET_TOK( TK_OC_AND ); }
"||" { setYyVal(TT_OC, TL_NONE); RET_TOK( TK_OC_OR ); }
"<<" { setYyVal(TT_OC, TL_NONE); RET_TOK( TK_OC_SL ); }
">>" { setYyVal(TT_OC, TL_NONE); RET_TOK( TK_OC_SR ); }
"%>%" { setYyVal(TT_OC, TL_NONE); RET_TOK( TK_OC_FORWARD_PIPE ); }
"%|%" { setYyVal(TT_OC, TL_NONE); RET_TOK( TK_OC_BASH_PIPE ); }

"false" { setYyVal(TT_LIT, TL_BOOL); RET_TOK( TK_LIT_FALSE ); }
"true" { setYyVal(TT_LIT, TL_BOOL); RET_TOK( TK_LIT_TRUE ); }
{litInt} { setYyVal(TT_LIT, TL_INT); RET_TOK( TK_LIT_INT ); }
{litFloat} { setYyVal(TT_LIT, TL_FLOAT); RET_TOK( TK_LIT_FLOAT ); }

\'.\' { setYyVal(TT_LIT, TL_CHAR); RET_TOK(TK_LIT_CHAR); }
\'.. { RET_TOK( TOKEN_ERRO ); }
\".*\" { setYyVal(TT_LIT, TL_STRING); RET_TOK(TK_LIT_STRING); }

{litInt}{identificador} { RET_TOK(TOKEN_ERRO); }
{litFloat}{identificador} { RET_TOK(TOKEN_ERRO); }

{identificador} { setYyVal(TT_ID, TL_NONE); RET_TOK(TK_IDENTIFICADOR); }

{especChar} { setYyVal(TT_CE, TL_NONE); RET_TOK(*yytext); }

\n {_LINE_NUMBER_UNIQUE_VAR_NAME++; _IS_LINE_COMMENT = 0;}

"/*" {_IS_OTHER_COMMENT = 1;}
"*/" {_IS_OTHER_COMMENT = 0;}
"//" {_IS_LINE_COMMENT = 1;}

" " { }
\t { }

. { RET_TOK( TOKEN_ERRO ); }


%%

int get_line_number(){
  return _LINE_NUMBER_UNIQUE_VAR_NAME;
}
