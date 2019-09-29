%{
#include <stdio.h>
#include <string.h>
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

%type<nodo_arvore> programa declFunc primType staticType literal operando expr comandoFuncExpr listaArgs argumento blocoComando listaParams parametro comando
comandoAtrib comandoBreak comandoShift comandoReturn comandoContinue comandoChamadaFunc comandoControleFluxo listaComandos listaForComandos forComando

%type<valor_lexico> TK_IDENTIFICADOR TK_LIT_INT TK_LIT_CHAR TK_LIT_FLOAT
TK_LIT_FALSE TK_LIT_TRUE TK_LIT_STRING TK_OC_EQ TK_OC_LE TK_OC_GE
TK_OC_NE TK_OC_AND TK_OC_OR TK_OC_BASH_PIPE TK_OC_FORWARD_PIPE TK_OC_SL TK_OC_SR
TK_PR_STATIC TK_PR_CONST TK_PR_INT TK_PR_CHAR TK_PR_FLOAT TK_PR_BOOL TK_PR_STRING
TK_PR_RETURN TK_PR_BREAK TK_PR_IF TK_PR_FOR TK_PR_WHILE TK_PR_CONTINUE
'+' '-' '*' '/' '!' '?' '&' '#' '%' '|' '^' '<' '>' ':' '=' '(' ')' '[' ']'

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

programa: declVarGlobal programa {$$ = NULL; arvore = $$;}; //Nao precisa
programa: declFunc programa {$$ = createNode($1->valor_lexico, 2); $$->valor_lexico.valTokStr = strdup($$->valor_lexico.valTokStr); addFilho($$, $1); addFilho($$, $2); arvore = $$;};
programa: {$$ = NULL; arvore = $$;};

declVarGlobal: staticType TK_IDENTIFICADOR ';'
|        staticType TK_IDENTIFICADOR '[' TK_LIT_INT ']' ';';

staticType: TK_PR_STATIC primType {$$ = createNode($1, 1); addFilho($$, $2);}
|            primType {$$ = $1;};

primType: TK_PR_INT {$$ = createNode($1, 0);}
| TK_PR_CHAR {$$ = createNode($1, 0);}
| TK_PR_BOOL {$$ = createNode($1, 0);}
| TK_PR_STRING {$$ = createNode($1, 0);}
| TK_PR_FLOAT {$$ = createNode($1, 0);};


declFunc: staticType TK_IDENTIFICADOR '(' listaParams ')' blocoComando {$$ = createNode($2, 4); $$->valor_lexico.valTokStr = strdup($$->valor_lexico.valTokStr); addFilho($$, $1);
                                                                        addFilho($$, createNode($2, 0));
                                                                        addFilho($$, $4);
                                                                        addFilho($$, $6);};

listaParams: parametro {$$ = $1;}
|            parametro ',' listaParams {$$ = createNode($1->valor_lexico, 2); $$->valor_lexico.valTokStr = strdup($$->valor_lexico.valTokStr); addFilho($$, $1); addFilho($$, $3);}
| {$$ = createNode(yylval.valor_lexico, 0);} ;

parametro: TK_PR_CONST primType TK_IDENTIFICADOR {$$ = createNode($3, 3);
                                                  addFilho($$, createNode($1, 0));
                                                  addFilho($$, $2);
                                                  addFilho($$, createNode($3, 0));}
|          primType TK_IDENTIFICADOR {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, createNode($2, 0));};

blocoComando: '{' listaComandos '}' {$$ = $2;} ;

listaComandos: comando listaComandos {$$ = createNode(yylval.valor_lexico, 2); addFilho($$, $1); addFilho($$, $2);}
| {$$ = createNode(yylval.valor_lexico, 0);} ;

comando: blocoComando {$$ = $1;}
|        declVarLocal {$$ = NULL;} //Precisa?
|        comandoAtrib ';' {$$ = $1;}
|        comandoEntradaSaida {$$ = NULL;} //Nao precisa
|        comandoChamadaFunc {$$ = $1;}
|        comandoShift ';' {$$ = $1;}
|        comandoReturn ';' {$$ = $1;}
|        comandoBreak ';' {$$ = $1;}
|        comandoContinue ';' {$$ = $1;}
|        comandoControleFluxo {$$ = $1;};

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

comandoAtrib: TK_IDENTIFICADOR '=' expr {$$ = createNode($2, 2); $$->valor_lexico.valTokStr = strdup("="); addFilho($$, createNode($1, 0)); addFilho($$, $3);}
|             TK_IDENTIFICADOR '[' expr ']' '=' expr {$$ = createNode($5, 2); $$->valor_lexico.valTokStr = strdup("=");
                                                          addFilho($$, createNode($2, 2));
                                                          $$->filhos[0]->valor_lexico.valTokStr = strdup("[]");
                                                          addFilho($$->filhos[0], createNode($1, 0));
                                                          addFilho($$->filhos[0], $3);
                                                          addFilho($$, $6);};

comandoEntradaSaida: TK_PR_INPUT expr ';'
|                    TK_PR_OUTPUT expr ';';

comandoChamadaFunc: TK_IDENTIFICADOR '(' listaArgs ')' ';' {$$ = createNode($1, 2); addFilho($$, createNode($1, 0)); addFilho($$, $3);}

listaArgs: argumento {$$ = $1;}
|          argumento ',' listaArgs {$$ = createNode($1->valor_lexico, 2); $$->valor_lexico.valTokStr = strdup($$->valor_lexico.valTokStr); addFilho($$, $1); addFilho($$, $3);}
| {$$ = createNode(yylval.valor_lexico, 0);};

argumento: expr {$$ = $1;};

comandoShift: TK_IDENTIFICADOR TK_OC_SL expr {$$ = createNode($2, 2); addFilho($$, createNode($1, 0)); addFilho($$, $3);}
|             TK_IDENTIFICADOR '[' expr ']' TK_OC_SL expr {$$ = createNode($5, 3); addFilho($$, createNode($1, 0)); addFilho($$, $3); addFilho($$, $6);}
|             TK_IDENTIFICADOR TK_OC_SR expr {$$ = createNode($2, 2); addFilho($$, createNode($1, 0)); addFilho($$, $3);}
|             TK_IDENTIFICADOR '[' expr ']' TK_OC_SR expr {$$ = createNode($5, 3); addFilho($$, createNode($1, 0)); addFilho($$, $3); addFilho($$, $6);};

comandoReturn: TK_PR_RETURN expr {$$ = createNode($1, 1); addFilho($$, $2);};

comandoBreak: TK_PR_BREAK {$$ = createNode($1, 0);};

comandoContinue: TK_PR_CONTINUE {$$ = createNode($1, 0);};

comandoControleFluxo: TK_PR_IF '(' expr ')' blocoComando {$$ = createNode($1, 2); addFilho($$, $3); addFilho($$, $5);}
|                     TK_PR_IF '(' expr ')' blocoComando TK_PR_ELSE blocoComando {$$ = createNode($1, 3); addFilho($$, $3); addFilho($$, $5); addFilho($$, $7);}
|                     TK_PR_FOR '(' listaForComandos ':' expr ':' listaForComandos ')' blocoComando {$$ = createNode($1, 4);
                                                                                                     addFilho($$, $3);
                                                                                                     addFilho($$, $5);
                                                                                                     addFilho($$, $7);
                                                                                                     addFilho($$, $9);}
|                     TK_PR_WHILE '(' expr ')' TK_PR_DO blocoComando {$$ = createNode($1, 2); addFilho($$, $3); addFilho($$, $6);};

listaForComandos: forComando {$$ = $1;}
|                 forComando ',' listaForComandos {$$ = createNode($1->valor_lexico, 2); $$->valor_lexico.valTokStr = strdup($$->valor_lexico.valTokStr); addFilho($$, $1); addFilho($$, $3);};

forComando: blocoComando {$$ = $1;}
|           comandoReturn {$$ = $1;}
|           comandoBreak {$$ = $1;}
|           comandoContinue {$$ = $1;}
|           comandoAtrib {$$ = $1;}
|           comandoShift {$$ = $1;};


expr: operando {$$ = $1;}
|     '(' expr ')' {$$ = $2;}
|     '+' expr     {$$ = createNode($1, 1); $$->valor_lexico.valTokStr = strdup("+"); addFilho($$, $2);} %prec UNARY_PLUS
|     '-' expr     {$$ = createNode($1, 1); $$->valor_lexico.valTokStr = strdup("-"); addFilho($$, $2);}                %prec UNARY_MINUS
|     '!' expr     {$$ = createNode($1, 1); $$->valor_lexico.valTokStr = strdup("!"); addFilho($$, $2);}
|     '&' expr     {$$ = createNode($1, 1); $$->valor_lexico.valTokStr = strdup("&"); addFilho($$, $2);}                %prec ADDRESS_OF
|     '*' expr     {$$ = createNode($1, 1); $$->valor_lexico.valTokStr = strdup("*"); addFilho($$, $2);}                %prec DEREFERENCE
|     '?' expr     {$$ = createNode($1, 1); $$->valor_lexico.valTokStr = strdup("?"); addFilho($$, $2);}                %prec EVAL_EXPR
|     '#' expr     {$$ = createNode($1, 1); $$->valor_lexico.valTokStr = strdup("#"); addFilho($$, $2);}
|     expr '+' expr {$$ = createNode($2, 2); $$->valor_lexico.valTokStr = strdup("+"); addFilho($$, $1); addFilho($$, $3); }
|     expr '-' expr {$$ = createNode($2, 2); $$->valor_lexico.valTokStr = strdup("-"); addFilho($$, $1); addFilho($$, $3); }
|     expr '*' expr {$$ = createNode($2, 2); $$->valor_lexico.valTokStr = strdup("*"); addFilho($$, $1); addFilho($$, $3); }
|     expr '/' expr {$$ = createNode($2, 2); $$->valor_lexico.valTokStr = strdup("/"); addFilho($$, $1); addFilho($$, $3); }
|     expr '%' expr {$$ = createNode($2, 2); $$->valor_lexico.valTokStr = strdup("%"); addFilho($$, $1); addFilho($$, $3); }
|     expr '|' expr {$$ = createNode($2, 2); $$->valor_lexico.valTokStr = strdup("|"); addFilho($$, $1); addFilho($$, $3); }
|     expr '&' expr {$$ = createNode($2, 2); $$->valor_lexico.valTokStr = strdup("&"); addFilho($$, $1); addFilho($$, $3); }
|     expr '^' expr {$$ = createNode($2, 2); $$->valor_lexico.valTokStr = strdup("^"); addFilho($$, $1); addFilho($$, $3); }
|     expr '<' expr {$$ = createNode($2, 2); $$->valor_lexico.valTokStr = strdup("<"); addFilho($$, $1); addFilho($$, $3); }
|     expr '>' expr {$$ = createNode($2, 2); $$->valor_lexico.valTokStr = strdup(">"); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_LE expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_GE expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_EQ expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_NE expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_AND expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_OR expr {$$ = createNode($2, 2); addFilho($$, $1); addFilho($$, $3); }
|     expr '?' expr ':' expr {$$ = createNode($4, 3); $$->valor_lexico.valTokStr = strdup("?:"); addFilho($$, $1); addFilho($$, $3); addFilho($$, $5);} ;

operando: TK_IDENTIFICADOR {$$ = createNode($1, 0);}
|         TK_IDENTIFICADOR '[' expr ']' {$$ = createNode($2, 2); $$->valor_lexico.valTokStr = strdup("[]");
                                         addFilho($$, createNode($1, 0));
                                         addFilho($$, $3);}

|         literal {$$ = $1;}
|         comandoFuncExpr {$$ = $1;};

comandoFuncExpr: TK_IDENTIFICADOR '(' listaArgs ')' {$$ = createNode($1, 2); addFilho($$, createNode($1, 0)); addFilho($$, $3);};

%%

void yyerror (char const *s) {
  printf("Linha %d:\n", get_line_number());
  printf("---> %s\n", s);
}

void exporta(void *head) {
  printArvore(head);
  FILE *fp = fopen("e3.csv", "w");
  exporta_arvore((NODO_ARVORE*) head, fp);
  fclose(fp);
}

void libera(void *head) {
  libera_arvore((NODO_ARVORE*) head);
}
