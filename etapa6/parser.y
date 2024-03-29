%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "include/defines.h"
#include "include/arvore.h"
#include "include/tabela.h"
#include "include/labeltable.h"
#include "include/iloc.h"

int yylex(void);
void yyerror (char const *s);
int get_line_number();

void exporta(void *head);
void libera(void *head);

//Tabela de simbolos
void addSimbolo(struct valLex valorL, TIPO_COMPOSTO tipo, int tipo_id, ARG_LIST* args, NODO_ARVORE* dimensions);

//Codigo ILOC
void genIlocCode(NODO_ARVORE* head);

ARG_LIST* listaArgsNovoEscopo;
TIPO_COMPOSTO tipoNovoEscopo;

T_SIMBOLO* tabelaSimbolos = NULL;

LABEL_TABLE* label_table = NULL;
void addLabTab(char* fName);

int lastFuncDesloc = 0;

struct valLex DUMB_VALEX;

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

  typedef struct node_bool_op{
    char *lt;
    char *lf;
  } BOOL_EXP_HEIR_ATT;

  typedef struct il_inst{
    char *inst;
  } ILOC_INST;

  typedef struct list_iloc_inst{
    struct list_iloc_inst *prox;

    ILOC_INST* instruction;
  } ILOC_INST_LIST;

  typedef struct arvore_t{
    struct arvore_t** filhos;
    int nFilhos;
    int nFilhosMax;

    struct valLex valor_lexico;

    int tipo;

    ILOC_INST_LIST* instructionList;
    char *IlocRegName;
    BOOL_EXP_HEIR_ATT bexpHatt;

  } NODO_ARVORE;

  typedef struct tipo_composto{
    int tipoPrim;
    int isStatic;
    int isConst;
  } TIPO_COMPOSTO;

  typedef struct argl{
    struct argl *prox;

    char *arg;
    TIPO_COMPOSTO tipoArg;
    int linhaArg;
  } ARG_LIST;

}

%union {
  struct valLex valor_lexico;
  NODO_ARVORE* nodo_arvore;
  TIPO_COMPOSTO tipoComposto;
  ARG_LIST* argList;
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

%type<nodo_arvore> programa declFunc declVarLocal listaDim listaDimExpr literal operando expr comandoFuncExpr listaArgs argumento blocoComando blocoComandoFun comando
comandoAtrib comandoBreak comandoShift comandoReturn comandoContinue comandoChamadaFunc comandoControleFluxo listaComandos listaForComandos forComando

%type<valor_lexico> TK_IDENTIFICADOR TK_LIT_INT TK_LIT_CHAR TK_LIT_FLOAT
TK_LIT_FALSE TK_LIT_TRUE TK_LIT_STRING TK_OC_EQ TK_OC_LE TK_OC_GE
TK_OC_NE TK_OC_AND TK_OC_OR TK_OC_BASH_PIPE TK_OC_FORWARD_PIPE TK_OC_SL TK_OC_SR
TK_PR_STATIC TK_PR_CONST TK_PR_INT TK_PR_CHAR TK_PR_FLOAT TK_PR_BOOL TK_PR_STRING
TK_PR_RETURN TK_PR_BREAK TK_PR_IF TK_PR_FOR TK_PR_WHILE TK_PR_CONTINUE
'+' '-' '*' '/' '!' '?' '&' '#' '%' '|' '^' '<' '>' ':' '=' '(' ')' '[' ']'

%type<tipoComposto> primType staticType tipoVarLocal

%type<argList> listaParams parametro

/* menor precedência */
%right '?' ':'
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

programa: declVarGlobal programa {$$ = $2; arvore = $$;};
programa: declFunc programa {$$ = $1; addFilho($$, $2); arvore = $$;};
programa: {$$ = NULL; arvore = $$;};

declVarGlobal: staticType TK_IDENTIFICADOR ';' { addSimbolo($2, $1, TID_VAR, NULL, NULL); free($2.valTokStr);}
|        staticType TK_IDENTIFICADOR listaDim ';' { addSimbolo($2, $1, TID_VET, NULL, $3); free($2.valTokStr);};

listaDim: '[' TK_LIT_INT ']' {$$ = createNode(DUMB_VALEX, 2); $$->valor_lexico.valTokStr = "[]"; addFilho($$, NULL); addFilho($$, createNode($2, 0));}
|         listaDim '[' TK_LIT_INT ']' {$$ = createNode(DUMB_VALEX, 2); $$->valor_lexico.valTokStr = "[]"; addFilho($$, $1); addFilho($$, createNode($3, 0));}

staticType: TK_PR_STATIC primType {$$ = $2; $$.isStatic = 1;}
|            primType {$$ = $1;};

primType: TK_PR_INT {$$ = (TIPO_COMPOSTO) {TL_INT, 0, 0};}
| TK_PR_CHAR {$$ = (TIPO_COMPOSTO) {TL_CHAR, 0, 0};}
| TK_PR_BOOL {$$ = (TIPO_COMPOSTO) {TL_BOOL, 0, 0};}
| TK_PR_STRING {$$ = (TIPO_COMPOSTO) {TL_STRING, 0, 0};}
| TK_PR_FLOAT {$$ = (TIPO_COMPOSTO) {TL_FLOAT, 0, 0};};

declFunc: staticType TK_IDENTIFICADOR '(' listaParams {listaArgsNovoEscopo = $4; tipoNovoEscopo = $1;} ')' {addSimbolo($2, $1, TID_FUNC, $4, NULL);} blocoComandoFun {addLabTab($2.valTokStr); $$ = createNode($2, 2); addFilho($$, $8);};

listaParams: parametro {$$ = $1; $$->prox = NULL;}
|            parametro ',' listaParams {$$ = $1; $$->prox = $3;}
| {$$ = NULL;};

parametro: TK_PR_CONST primType TK_IDENTIFICADOR {$$ = malloc(sizeof(ARG_LIST)); $$->tipoArg = $2; $$->tipoArg.isConst = 1; $$->arg = strdup($3.valTokStr); $$->linhaArg = $3.line_number; free($3.valTokStr);}
|          primType TK_IDENTIFICADOR {$$ = malloc(sizeof(ARG_LIST)); $$->tipoArg = $1; $$->arg = strdup($2.valTokStr); $$->linhaArg = $2.line_number; free($2.valTokStr);} ;

blocoComandoFun: '{' {pushEscopo(tabelaSimbolos, listaArgsNovoEscopo, tipoNovoEscopo); listaArgsNovoEscopo = NULL; tipoNovoEscopo.tipoPrim = TL_NONE;} listaComandos '}' {$$ = $3; print_tabela(tabelaSimbolos); infere_tipos($$, NULL, tabelaSimbolos); genNodeCode($$, tabelaSimbolos); lastFuncDesloc = popEscopo(tabelaSimbolos);} ;
blocoComando: '{' {pushEscopo(tabelaSimbolos, listaArgsNovoEscopo, tipoNovoEscopo); listaArgsNovoEscopo = NULL; tipoNovoEscopo.tipoPrim = TL_NONE;} listaComandos '}' {$$ = createNode(DUMB_VALEX, 1); addFilho($$, $3); $$->valor_lexico.valTokStr = strdup("{}"); print_tabela(tabelaSimbolos); infere_tipos($$, NULL, tabelaSimbolos); genNodeCode($$, tabelaSimbolos); popEscopo(tabelaSimbolos);} ;

listaComandos: comando listaComandos {if($$ != NULL) {$$ = $1; addFilho($$, $2);} else $$ = $2;}
| {$$ = NULL;} ;

comando: blocoComando ';' {$$ = $1;}
|        declVarLocal {$$ = $1;}
|        comandoAtrib ';' {$$ = $1;}
|        comandoEntradaSaida {$$ = NULL;}
|        comandoChamadaFunc {$$ = $1;}
|        comandoShift ';' {$$ = $1;}
|        comandoReturn ';' {$$ = $1;}
|        comandoBreak ';' {$$ = $1;}
|        comandoContinue ';' {$$ = $1;}
|        comandoControleFluxo ';' {$$ = $1;};

declVarLocal: tipoVarLocal TK_IDENTIFICADOR ';' {addSimbolo($2, $1, TID_VAR, NULL, NULL); $$ = NULL; free($2.valTokStr);}
|             tipoVarLocal TK_IDENTIFICADOR TK_OC_LE literal ';' {addSimbolo($2, $1, TID_VAR, NULL, NULL); $$ = createNode($3, 3); $$->valor_lexico.valTokStr = strdup("="); addFilho($$, createNode($2, 0)); addFilho($$, $4); free($3.valTokStr);}
|             tipoVarLocal TK_IDENTIFICADOR TK_OC_LE TK_IDENTIFICADOR ';' {addSimbolo($2, $1, TID_VAR, NULL, NULL); $$ = createNode($3, 3); $$->valor_lexico.valTokStr = strdup("="); addFilho($$, createNode($2, 0)); addFilho($$, createNode($4, 1)); free($3.valTokStr);};

tipoVarLocal: primType {$$ = $1;}
|             TK_PR_STATIC primType {$$ = $2; $$.isStatic = 1;}
|             TK_PR_CONST primType {$$ = $2; $$.isConst = 1;}
|             TK_PR_STATIC TK_PR_CONST primType {$$ = $3; $$.isStatic = 1; $$.isConst = 1;};

literal: TK_LIT_INT { $$ = createNode($1, 1); $$->tipo = TL_INT; addSimbolo($1, (TIPO_COMPOSTO) {TL_INT, 0, 0}, -1, NULL, NULL);}
|        TK_LIT_CHAR { $$ = createNode($1, 1); $$->tipo = TL_CHAR; addSimbolo($1, (TIPO_COMPOSTO) {TL_CHAR, 0, 0}, -1, NULL, NULL);}
|        TK_LIT_TRUE { $$ = createNode($1, 1); $$->tipo = TL_BOOL; addSimbolo($1, (TIPO_COMPOSTO) {TL_BOOL, 0, 0}, -1, NULL, NULL);}
|        TK_LIT_FALSE { $$ = createNode($1, 1); $$->tipo = TL_BOOL; addSimbolo($1, (TIPO_COMPOSTO) {TL_BOOL, 0, 0}, -1, NULL, NULL);}
|        TK_LIT_FLOAT { $$ = createNode($1, 1); $$->tipo = TL_FLOAT; addSimbolo($1, (TIPO_COMPOSTO) {TL_FLOAT, 0, 0}, -1, NULL, NULL);}
|        TK_LIT_STRING { $$ = createNode($1, 1); $$->tipo = TL_STRING; addSimbolo($1, (TIPO_COMPOSTO) {TL_STRING, 0, 0}, -1, NULL, NULL);};

comandoAtrib: TK_IDENTIFICADOR '=' expr {$$ = createNode($2, 3); $$->valor_lexico.valTokStr = strdup("="); addFilho($$, createNode($1, 0)); addFilho($$, $3);}
|             TK_IDENTIFICADOR listaDimExpr '=' expr {$$ = createNode($3, 3); $$->valor_lexico.valTokStr = strdup("=");
                                                          addFilho($$, createNode($3, 2));
                                                          $$->filhos[0]->valor_lexico.valTokStr = strdup("[]");
                                                          addFilho($$->filhos[0], createNode($1, 0));
                                                          addFilho($$->filhos[0], $2);
                                                          addFilho($$, $4);};

comandoEntradaSaida: TK_PR_INPUT expr ';' {libera_arvore($2);}
|                    TK_PR_OUTPUT expr ';' {libera_arvore($2);};

comandoChamadaFunc: TK_IDENTIFICADOR '(' listaArgs ')' ';' {$$ = createNode($1, 2); addFilho($$, $3);}

listaArgs: argumento {$$ = $1;}
|          argumento ',' listaArgs {$$ = $1; addFilho($$, $3);}
| {$$ = NULL;};

argumento: expr {$$ = $1;};

comandoShift: TK_IDENTIFICADOR TK_OC_SL expr {$$ = createNode($2, 3); addFilho($$, createNode($1, 0)); addFilho($$, $3);}
|             TK_IDENTIFICADOR '[' expr ']' TK_OC_SL expr {$$ = createNode($5, 3);
                                                          addFilho($$, createNode($2, 2));
                                                          $$->filhos[0]->valor_lexico.valTokStr = strdup("[]");
                                                          addFilho($$->filhos[0], createNode($1, 0));
                                                          addFilho($$->filhos[0], $3);
                                                          addFilho($$, $6);};
|             TK_IDENTIFICADOR TK_OC_SR expr {$$ = createNode($2, 3); addFilho($$, createNode($1, 0)); addFilho($$, $3);}
|             TK_IDENTIFICADOR '[' expr ']' TK_OC_SR expr {$$ = createNode($5, 3);
                                                          addFilho($$, createNode($2, 2));
                                                          $$->filhos[0]->valor_lexico.valTokStr = strdup("[]");
                                                          addFilho($$->filhos[0], createNode($1, 0));
                                                          addFilho($$->filhos[0], $3);
                                                          addFilho($$, $6);};

comandoReturn: TK_PR_RETURN expr {$$ = createNode($1, 2); $$->valor_lexico.valTokStr = strdup("return"); addFilho($$, $2);};

comandoBreak: TK_PR_BREAK {$$ = createNode($1, 1); $$->valor_lexico.valTokStr = strdup("break");};

comandoContinue: TK_PR_CONTINUE {$$ = createNode($1, 1); $$->valor_lexico.valTokStr = strdup("continue");};

comandoControleFluxo: TK_PR_IF '(' expr ')' blocoComando {$$ = createNode($1, 3); $$->valor_lexico.valTokStr = strdup("if"); addFilho($$, $3); addFilho($$, $5);}
|                     TK_PR_IF '(' expr ')' blocoComando TK_PR_ELSE blocoComando {$$ = createNode($1, 4); $$->valor_lexico.valTokStr = strdup("if"); addFilho($$, $3); addFilho($$, $5); addFilho($$, $7);}
|                     TK_PR_FOR '(' listaForComandos ':' expr ':' listaForComandos ')' blocoComando {$$ = createNode($1, 5); $$->valor_lexico.valTokStr = strdup("for");
                                                                                                     addFilho($$, $3);
                                                                                                     addFilho($$, $5);
                                                                                                     addFilho($$, $7);
                                                                                                     addFilho($$, $9);}
|                     TK_PR_WHILE '(' expr ')' TK_PR_DO blocoComando {$$ = createNode($1, 3); $$->valor_lexico.valTokStr = strdup("while"); addFilho($$, $3); addFilho($$, $6);};

listaForComandos: forComando {$$ = $1;}
|                 forComando ',' listaForComandos {$$ = $1; addFilho($$, $3);};

forComando: blocoComando {$$ = $1;}
|           comandoReturn {$$ = $1;}
|           comandoBreak {$$ = $1;}
|           comandoContinue {$$ = $1;}
|           comandoAtrib {$$ = $1;}
|           comandoShift {$$ = $1;};

expr: operando {$$ = $1;}
|     '(' expr ')' {$$ = $2;}
|     '+' expr     {$$ = createNode($1, 2); $$->valor_lexico.valTokStr = strdup("+"); addFilho($$, $2);} %prec UNARY_PLUS
|     '-' expr     {$$ = createNode($1, 2); $$->valor_lexico.valTokStr = strdup("-"); addFilho($$, $2);}                %prec UNARY_MINUS
|     '!' expr     {$$ = createNode($1, 2); $$->valor_lexico.valTokStr = strdup("!"); addFilho($$, $2);}
|     '&' expr     {$$ = createNode($1, 2); $$->valor_lexico.valTokStr = strdup("&"); addFilho($$, $2);}                %prec ADDRESS_OF
|     '*' expr     {$$ = createNode($1, 2); $$->valor_lexico.valTokStr = strdup("*"); addFilho($$, $2);}                %prec DEREFERENCE
|     '?' expr     {$$ = createNode($1, 2); $$->valor_lexico.valTokStr = strdup("?"); addFilho($$, $2);}                %prec EVAL_EXPR
|     '#' expr     {$$ = createNode($1, 2); $$->valor_lexico.valTokStr = strdup("#"); addFilho($$, $2);}
|     expr '+' expr {$$ = createNode($2, 3); $$->valor_lexico.valTokStr = strdup("+"); addFilho($$, $1); addFilho($$, $3); }
|     expr '-' expr {$$ = createNode($2, 3); $$->valor_lexico.valTokStr = strdup("-"); addFilho($$, $1); addFilho($$, $3); }
|     expr '*' expr {$$ = createNode($2, 3); $$->valor_lexico.valTokStr = strdup("*"); addFilho($$, $1); addFilho($$, $3); }
|     expr '/' expr {$$ = createNode($2, 3); $$->valor_lexico.valTokStr = strdup("/"); addFilho($$, $1); addFilho($$, $3); }
|     expr '%' expr {$$ = createNode($2, 3); $$->valor_lexico.valTokStr = strdup("%"); addFilho($$, $1); addFilho($$, $3); }
|     expr '|' expr {$$ = createNode($2, 3); $$->valor_lexico.valTokStr = strdup("|"); addFilho($$, $1); addFilho($$, $3); }
|     expr '&' expr {$$ = createNode($2, 3); $$->valor_lexico.valTokStr = strdup("&"); addFilho($$, $1); addFilho($$, $3); }
|     expr '^' expr {$$ = createNode($2, 3); $$->valor_lexico.valTokStr = strdup("^"); addFilho($$, $1); addFilho($$, $3); }
|     expr '<' expr {$$ = createNode($2, 3); $$->valor_lexico.valTokStr = strdup("<"); addFilho($$, $1); addFilho($$, $3); }
|     expr '>' expr {$$ = createNode($2, 3); $$->valor_lexico.valTokStr = strdup(">"); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_LE expr {$$ = createNode($2, 3); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_GE expr {$$ = createNode($2, 3); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_EQ expr {$$ = createNode($2, 3); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_NE expr {$$ = createNode($2, 3); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_AND expr {$$ = createNode($2, 3); addFilho($$, $1); addFilho($$, $3); }
|     expr TK_OC_OR expr {$$ = createNode($2, 3); addFilho($$, $1); addFilho($$, $3); }
|     expr '?' expr ':' expr {$$ = createNode($4, 3); $$->valor_lexico.valTokStr = strdup("?:"); addFilho($$, $1); addFilho($$, $3); addFilho($$, $5);} ;

operando: TK_IDENTIFICADOR {$$ = createNode($1, 1);}
|         TK_IDENTIFICADOR listaDimExpr {$$ = createNode(DUMB_VALEX, 3); $$->valor_lexico.valTokStr = strdup("[]");
                                         addFilho($$, createNode($1, 0));
                                         addFilho($$, $2);}

|         literal {$$ = $1;}
|         comandoFuncExpr {$$ = $1;};

listaDimExpr: '[' expr ']' {$$ = $2;}
|             listaDimExpr '[' expr ']' {$$ = $1; addFilho($$, $3);};

comandoFuncExpr: TK_IDENTIFICADOR '(' listaArgs ')' {$$ = createNode($1, 2); addFilho($$, $3);};

%%

void yyerror (char const *s) {
  printf("Linha %d:\n", get_line_number());
  printf("---> %s\n", s);
  exit(0);
}

void genIlocCode(NODO_ARVORE* head){
  genSaidaIloc(head, tabelaSimbolos);
}

void exporta(void *head) {

  if(DEBUG_MODE){
    printf("AST:\n\n");
    printArvore(head, 0);
    printf("\n\nTabela de Símbolos Global:\n");
    print_tabela(tabelaSimbolos);
  }

  FILE *fp = fopen("e3.csv", "w");

  NODO_ARVORE* raiz = (NODO_ARVORE*) head;

  int filhosNotNullRaiz = 0;

  if(raiz != NULL){
    for(int i = 0; i < raiz->nFilhosMax; i++)
      if(raiz->filhos[i] != NULL)
        filhosNotNullRaiz++;

    if(filhosNotNullRaiz == 0)
      fprintf(fp, "%p, \n", raiz);
  }

  if(filhosNotNullRaiz > 0)
    exporta_arvore(raiz, fp);

  fclose(fp);

  genIlocCode((NODO_ARVORE*) head);
}

void addLabTab(char* fName){
  if(label_table == NULL)
    label_table = make_label_table();

  char buffer[128];
  sprintf(buffer, "L%s", fName);
  insere_label_table(label_table, fName, buffer, lastFuncDesloc);
}

void libera(void *head) {
  libera_arvore((NODO_ARVORE*) head);
  free_tabela_recursive(tabelaSimbolos);
  tabelaSimbolos = NULL;
}

void addSimbolo(struct valLex valorL, TIPO_COMPOSTO tipo, int tipo_id, ARG_LIST* args, NODO_ARVORE* dimensions){
  if(tabelaSimbolos == NULL)
    tabelaSimbolos = make_tabela();

  S_INFO sInfo;

  sInfo.linha = valorL.line_number;
  sInfo.tipo = tipo;
  sInfo.argList = args;
  sInfo.tipoIdentificador = tipo_id;

  char *litToString = malloc(1000);

  if(valorL.tipo_token == TT_ID){
    sInfo.natureza = NATUREZA_IDENTIFICADOR;
    sInfo.idName = valorL.valTokStr;
  }else if(valorL.tipo_token == TT_LIT){

    switch(valorL.tipo_literal){
      case TL_INT:
        sInfo.natureza = NATUREZA_LITERAL_INT;
        sprintf(litToString, "%d", valorL.valTokInt);
        break;
      case TL_CHAR:
        sInfo.natureza = NATUREZA_LITERAL_CHAR;
        sprintf(litToString, "%c", valorL.valTokChar);
        break;
      case TL_BOOL:
        sInfo.natureza = NATUREZA_LITERAL_BOOL;
        if(valorL.valTokBool == 0)
          sprintf(litToString, "false");
        else
          sprintf(litToString, "true");
        break;
      case TL_FLOAT:
        sInfo.natureza = NATUREZA_LITERAL_FLOAT;
        sprintf(litToString, "%f", valorL.valTokFloat);
        break;
      case TL_STRING:
        sInfo.natureza = NATUREZA_LITERAL_STRING;
        if(strlen(valorL.valTokStr) >= 999){
          free(litToString);
          litToString = malloc(strlen(valorL.valTokStr) + 1);
        }
        sprintf(litToString, "%s", valorL.valTokStr);
        break;
    }

    sInfo.idName = litToString;
  }

  insere_tabela(tabelaSimbolos, sInfo, dimensions);

  free(litToString);
}
