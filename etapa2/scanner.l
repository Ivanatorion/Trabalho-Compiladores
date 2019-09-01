%{
#include <stdio.h>
#include "parser.tab.h"

int _LINE_NUMBER_UNIQUE_VAR_NAME = 1;
int _IS_LINE_COMMENT = 0;
int _IS_OTHER_COMMENT = 0;

#define RET_TOK(TOKEN) if(!_IS_LINE_COMMENT && !_IS_OTHER_COMMENT) return TOKEN;

%}

especChar [\[,\],\,\;\:\(\)\{\}\+\-\|\?\@\*\/\<\>\=\!\&\%\#\^\.\$\~\`]
digit [0-9]
litInt {digit}+
cExp [eE][-+]?{litInt}?
litFloat {litInt}?("."{litInt}){cExp}?

identificador [a-zA-Z_][a-zA-Z0-9_]*

%%

"int" { RET_TOK(TK_PR_INT); }
"float" { RET_TOK(TK_PR_FLOAT); }
"bool" { RET_TOK(TK_PR_BOOL); }
"char" { RET_TOK(TK_PR_CHAR); }
"string" { RET_TOK(TK_PR_STRING); }
"if" { RET_TOK(TK_PR_IF); }
"then" { RET_TOK(TK_PR_THEN); }
"else" { RET_TOK(TK_PR_ELSE); }
"while" { RET_TOK(TK_PR_WHILE); }
"do" { RET_TOK(TK_PR_DO); }
"input" { RET_TOK(TK_PR_INPUT); }
"output" { RET_TOK(TK_PR_OUTPUT); }
"return" { RET_TOK(TK_PR_RETURN); }
"const" { RET_TOK(TK_PR_CONST); }
"static" { RET_TOK(TK_PR_STATIC); }
"foreach" { RET_TOK(TK_PR_FOREACH); }
"for" { RET_TOK(TK_PR_FOR); }
"switch" { RET_TOK(TK_PR_SWITCH); }
"case" { RET_TOK(TK_PR_CASE); }
"break" { RET_TOK(TK_PR_BREAK); }
"continue" { RET_TOK(TK_PR_CONTINUE); }
"class" { RET_TOK(TK_PR_CLASS); }
"private" { RET_TOK(TK_PR_PRIVATE); }
"public" { RET_TOK(TK_PR_PUBLIC); }
"protected" { RET_TOK(TK_PR_PROTECTED); }
"end" { RET_TOK(TK_PR_END); }

"<=" { RET_TOK( TK_OC_LE ); }
">=" { RET_TOK( TK_OC_GE ); }
"==" { RET_TOK( TK_OC_EQ ); }
"!=" { RET_TOK( TK_OC_NE ); }
"&&" { RET_TOK( TK_OC_AND ); }
"||" { RET_TOK( TK_OC_OR ); }
"<<" { RET_TOK( TK_OC_SL ); }
">>" { RET_TOK( TK_OC_SR ); }
"%>%" { RET_TOK( TK_OC_FORWARD_PIPE ); }
"%|%" { RET_TOK( TK_OC_BASH_PIPE ); }

"false" { RET_TOK( TK_LIT_FALSE ); }
"true" { RET_TOK( TK_LIT_TRUE ); }
{litInt} { RET_TOK( TK_LIT_INT ); }
{litFloat} { RET_TOK( TK_LIT_FLOAT ); }

\'.\' { RET_TOK(TK_LIT_CHAR); }
\'.*\' { RET_TOK( TOKEN_ERRO ); }
\".*\" { RET_TOK(TK_LIT_STRING); }

{litInt}.?{identificador} { RET_TOK(TOKEN_ERRO); }
{litFloat}{identificador} { RET_TOK(TOKEN_ERRO); }

{identificador} { RET_TOK(TK_IDENTIFICADOR); }

{especChar} { RET_TOK(*yytext); }

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
