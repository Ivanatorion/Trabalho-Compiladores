%{
#include <stdio.h>
#include "include/defines.h"
#include "include/arvore.h"

int yylex(void);
void yyerror (char const *s);
int get_line_number();

void exporta(void *head);
void libera(void *head);

%}

%code requires {
  struct valLex{
    int line_number;
    int tipo_token;
    int tipo_literal;
    union{
      char *valTokStr;
      int valTokInt;
      int valTokBool;
      float valTokFloat;
      char valTokChar;
    };
  };

  typedef struct arvore_t{
    struct arvore_t** filhos;
    int nFilhos;

    struct valLex valor_lexico;
  } NODO_ARVORE;
}

%union {
  struct valLex valor_lexico;
  NODO_ARVORE* nodo_arvore;
}

%define parse.error verbose

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

%type<nodo_arvore> literal operando expr comandoFuncExpr listaArgs

%type<valor_lexico> TK_IDENTIFICADOR TK_LIT_INT TK_LIT_CHAR TK_LIT_FLOAT
TK_LIT_FALSE TK_LIT_TRUE TK_LIT_STRING TK_OC_EQ TK_OC_LE TK_OC_GE
TK_OC_NE TK_OC_AND TK_OC_OR TK_OC_BASH_PIPE TK_OC_FORWARD_PIPE
'+' '-' '*' '/' '!' '?' '&' '#' '%' '|' '^' '<' '>' ':'

/* menor precedência */
%right '?' ':'               /* operador ternário está certo? */
%left TK_OC_OR
%left TK_OC_AND
%left '|'
%left '^'
%left '&'
%left TK_OC_EQ TK_OC_NE
%left TK_OC_LE TK_OC_GE '<' '>'
%left TK_OC_SL  TK_OC_SR     /* precisa desta definição de associatividade? */
%left '+' '-'
%left '*' '/' '%'
%right '!' UNARY_PLUS UNARY_MINUS ADDRESS_OF DEREFERENCE EVAL_EXPR '#'
/* maior precedência */

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

literal: TK_LIT_INT { $$ = createNode($1, 0);}
|        TK_LIT_CHAR { $$ = createNode($1, 0);}
|        TK_LIT_TRUE { $$ = createNode($1, 0);}
|        TK_LIT_FALSE { $$ = createNode($1, 0);}
|        TK_LIT_FLOAT { $$ = createNode($1, 0);}
|        TK_LIT_STRING { $$ = createNode($1, 0);};

comandoAtrib: TK_IDENTIFICADOR '=' expr ';' {printArvore($3);}
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


expr: operando {$$ = $1;}
|     '(' expr ')' {$$ = $2;}
|     '+' expr     {$$ = createNode($1, 1); addFilho($$, $2);} %prec UNARY_PLUS
|     '-' expr     {$$ = createNode($1, 1); addFilho($$, $2);}                %prec UNARY_MINUS
|     '!' expr     {$$ = createNode($1, 1); addFilho($$, $2);}
|     '&' expr     {$$ = createNode($1, 1); addFilho($$, $2);}                %prec ADDRESS_OF
|     '*' expr     {$$ = createNode($1, 1); addFilho($$, $2);}                %prec DEREFERENCE
|     '?' expr     {$$ = createNode($1, 1); addFilho($$, $2);}                %prec EVAL_EXPR
|     '#' expr     {$$ = createNode($1, 1); addFilho($$, $2);}
|     expr '+' expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr '-' expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr '*' expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr '/' expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr '%' expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr '|' expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr '&' expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr '^' expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr '<' expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr '>' expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_LE expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_GE expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_EQ expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_NE expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_AND expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_OR expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr '?' expr ':' expr {$$ = createNode($4, 3); addFilho($$, $1); addFilho($$, $3); addFilho($$, $5);} ;

operando: TK_IDENTIFICADOR {$$ = createNode($1, 0);}
|         TK_IDENTIFICADOR '[' expr ']' {$$ = createNode(yylval.valor_lexico, 2);
                                         addFilho($$, createNode($1, 0));
                                         addFilho($$, $3);}

|         literal {$$ = $1;}
|         comandoFuncExpr;

comandoFuncExpr: TK_IDENTIFICADOR '(' listaArgs ')' {$$ = createNode($1, 2); addFilho($$, createNode($1, 0)); addFilho($$, $3);};

%%

void yyerror (char const *s) {
  printf("Linha %d:\n", get_line_number());
  printf("---> %s\n", s);
}

void exporta(void *head) {
  //printArvore(head);
}

void libera(void *head) {

}
