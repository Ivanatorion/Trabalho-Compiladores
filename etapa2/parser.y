%{
#include <stdio.h>

int yylex(void);
void yyerror (char const *s);
%}

%token TK_PR_INT
%token TK_PR_FLOAT
%token TK_PR_BOOL
%token TK_PR_CHAR
%token TK_PR_STRING
%token TK_PR_IF
%token TK_PR_THEN
%token TK_PR_ELSE
%token TK_PR_WHILE
%token TK_PR_DO
%token TK_PR_INPUT
%token TK_PR_OUTPUT
%token TK_PR_RETURN
%token TK_PR_CONST
%token TK_PR_STATIC
%token TK_PR_FOREACH
%token TK_PR_FOR
%token TK_PR_SWITCH
%token TK_PR_CASE
%token TK_PR_BREAK
%token TK_PR_CONTINUE
%token TK_PR_CLASS
%token TK_PR_PRIVATE
%token TK_PR_PUBLIC
%token TK_PR_PROTECTED
%token TK_PR_END
%token TK_PR_DEFAULT
%token TK_OC_LE
%token TK_OC_GE
%token TK_OC_EQ
%token TK_OC_NE
%token TK_OC_AND
%token TK_OC_OR
%token TK_OC_SL
%token TK_OC_SR
%token TK_OC_FORWARD_PIPE
%token TK_OC_BASH_PIPE
%token TK_LIT_INT
%token TK_LIT_FLOAT
%token TK_LIT_FALSE
%token TK_LIT_TRUE
%token TK_LIT_CHAR
%token TK_LIT_STRING
%token TK_IDENTIFICADOR
%token TOKEN_ERRO
%start programa
%%

programa: declVarGlobal programa;
programa: declFunc programa;
programa: ;



declVarGlobal: TK_IDENTIFICADOR staticType ';'
|        TK_IDENTIFICADOR '[' TK_LIT_INT ']' staticType ';';

staticType: TK_PR_STATIC primType
|            primType;

primType: TK_PR_INT | TK_PR_CHAR | TK_PR_BOOL
| TK_PR_STRING | TK_PR_FLOAT;



declFunc: staticType TK_IDENTIFICADOR '(' listaArgs ')' blocoComando;

listaArgs: argumento
|          argumento ',' listaArgs
| ;

blocoComando: '{' listaComandos '}'

argumento: TK_PR_CONST primType TK_IDENTIFICADOR
|          primType TK_IDENTIFICADOR;

listaComandos: comando listaComandos
| ;

comando: blocoComando
|        declVarLocal;

declVarLocal: tipoVarLocal TK_IDENTIFICADOR ';'
|             tipoVarLocal TK_IDENTIFICADOR TK_OC_LE literal ';';

literal: TK_LIT_INT
|        TK_LIT_CHAR
|        TK_LIT_TRUE
|        TK_LIT_FALSE
|        TK_LIT_FLOAT
|        TK_LIT_STRING;

tipoVarLocal: primType
|             TK_PR_STATIC primType
|             TK_PR_CONST primType
|             TK_PR_STATIC TK_PR_CONST primType;

%%

void yyerror (char const *s) {
  printf("Erro Sintatico (Linha %d)!\n", 1);
}
