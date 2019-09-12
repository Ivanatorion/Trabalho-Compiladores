%{
#include <stdio.h>

int yylex(void);
void yyerror (char const *s);
int get_line_number();
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

%left TK_OC_LE TK_OC_GE TK_OC_EQ TK_OC_NE TK_OC_AND TK_OC_OR TK_OC_SL TK_OC_SR
TK_OC_FORWARD_PIPE TK_OC_BASH_PIPE '<' '>' '+' '*' '/' '-' '&' '?' '%' '|' '^' '!' '#' ':'

%%

programa: declVarGlobal programa;
programa: declFunc programa;
programa: ;

declVarGlobal: staticType TK_IDENTIFICADOR ';'
|        staticType TK_IDENTIFICADOR '[' TK_LIT_INT ']' ';';

staticType: TK_PR_STATIC primType
|            primType;

primType: TK_PR_INT | TK_PR_CHAR | TK_PR_BOOL
| TK_PR_STRING | TK_PR_FLOAT;


declFunc: staticType TK_IDENTIFICADOR '(' listaParams ')' blocoComando;

listaParams: parametro
|            parametro ',' listaParams
| ;

parametro: TK_PR_CONST primType TK_IDENTIFICADOR
|          primType TK_IDENTIFICADOR;

blocoComando: '{' listaComandos '}'

listaComandos: comando listaComandos
| ;

comando: blocoComando
|        declVarLocal
|        comandoAtrib
|        comandoEntradaSaida
|        comandoChamadaFunc
|        comandoShift
|        comandoReturn
|        comandoBreak
|        comandoContinue
|        comandoControleFluxo;



declVarLocal: tipoVarLocal TK_IDENTIFICADOR ';'
|             tipoVarLocal TK_IDENTIFICADOR TK_OC_LE literal ';'
|             tipoVarLocal TK_IDENTIFICADOR TK_OC_LE TK_IDENTIFICADOR ';';

tipoVarLocal: primType
|             TK_PR_STATIC primType
|             TK_PR_CONST primType
|             TK_PR_STATIC TK_PR_CONST primType;

literal: TK_LIT_INT
|        TK_LIT_CHAR
|        TK_LIT_TRUE
|        TK_LIT_FALSE
|        TK_LIT_FLOAT
|        TK_LIT_STRING;


comandoAtrib: TK_IDENTIFICADOR '=' expr ';'
|             TK_IDENTIFICADOR '[' expr ']' '=' expr ';';




comandoEntradaSaida: TK_PR_INPUT expr ';'
|                    TK_PR_OUTPUT expr ';';

comandoChamadaFunc: TK_IDENTIFICADOR '(' listaArgs ')' ';'
|                   TK_IDENTIFICADOR '(' ')' ';';

listaArgs: argumento
|          argumento ',' listaArgs;

argumento: expr;

comandoShift: TK_IDENTIFICADOR TK_OC_SL expr ';'
|             TK_IDENTIFICADOR '[' expr ']' TK_OC_SL expr ';'
|             TK_IDENTIFICADOR TK_OC_SR expr ';'
|             TK_IDENTIFICADOR '[' expr ']' TK_OC_SR expr ';';



comandoReturn: TK_PR_RETURN expr ';';

comandoBreak: TK_PR_BREAK ';';

comandoContinue: TK_PR_CONTINUE ';';

comandoControleFluxo: TK_PR_IF '(' expr ')' blocoComando
|                     TK_PR_IF '(' expr ')' blocoComando TK_PR_ELSE blocoComando
|                     TK_PR_FOR '(' listaForComandos ':' expr ':' listaForComandos ')' blocoComando
|                     TK_PR_WHILE '(' expr ')' TK_PR_DO blocoComando;

listaForComandos: forComando
|                 forComando ',' listaForComandos;

forComando: blocoComando
|           comandoReturn
|           comandoBreak
|           comandoContinue
|           comandoAtrib
|           comandoShift;


expr: operando
|     '(' expr ')'
|     '+' expr
|     '-' expr
|     '!' expr
|     '&' expr
|     '*' expr
|     '?' expr
|     '#' expr
|     expr '+' expr
|     expr '-' expr
|     expr '*' expr
|     expr '/' expr
|     expr '%' expr
|     expr '|' expr
|     expr '&' expr
|     expr '^' expr
|     expr '<' expr
|     expr '>' expr
|     expr TK_OC_LE expr
|     expr TK_OC_GE expr
|     expr TK_OC_EQ expr
|     expr TK_OC_NE expr
|     expr TK_OC_AND expr
|     expr TK_OC_OR expr
|     expr '?' expr ':' expr;

operando: TK_IDENTIFICADOR
|         TK_IDENTIFICADOR '[' expr ']'
|         literal
|         comandoFuncExpr;

comandoFuncExpr: TK_IDENTIFICADOR '(' listaArgs ')'
|                TK_IDENTIFICADOR '(' ')';

%%

void yyerror (char const *s) {
  printf("Erro Sintatico (Linha %d)!\n", get_line_number());
}
