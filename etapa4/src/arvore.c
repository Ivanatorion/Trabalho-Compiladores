#include "../include/arvore.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

NODO_ARVORE* createNode(struct valLex valor, int nFilhosMax){
  NODO_ARVORE *nodo = malloc(sizeof(NODO_ARVORE));
  nodo->nFilhos = 0;
  nodo->nFilhosMax = nFilhosMax;

  if(nFilhosMax > 0){
    nodo->filhos = malloc(sizeof(NODO_ARVORE*) * nFilhosMax);
    for(int i = 0; i < nFilhosMax; i++)
      nodo->filhos[i] = NULL;
  }
  else
    nodo->filhos = NULL;

  nodo->valor_lexico = valor;
  nodo->tipo = TL_UNKNOWN;
  return nodo;
}

void addFilho(NODO_ARVORE* pai, NODO_ARVORE* filho){
  int i = pai->nFilhos;

  if(i == pai->nFilhosMax){
    if(i == 0)
      return;
    else if(pai->filhos[i-1] == NULL){
      pai->filhos[i-1] = filho;
    }
    else
      addFilho(pai->filhos[i-1], filho);
  }
  else{
    pai->filhos[i] = filho;
    pai->nFilhos++;
  }
}

void infere_tipos(NODO_ARVORE* arvore, T_SIMBOLO* tabela){
  if(arvore == NULL)
    return;

  for(int i = 0; i < arvore->nFilhosMax; i++)
    infere_tipos(arvore->filhos[i], tabela);

  if(arvore->tipo != TL_UNKNOWN)
    return;

  S_INFO sInfo;
  if(arvore->valor_lexico.tipo_token == TT_ID){
    if(consulta_tabela(tabela, arvore->valor_lexico.valTokStr, &sInfo) == ERR_UNDECLARED){
      printf("Erro (Linha %d): Identificador nao declarado (%s)\n", arvore->valor_lexico.line_number, arvore->valor_lexico.valTokStr);
      exit(ERR_UNDECLARED);
    }

    arvore->tipo = sInfo.tipo.tipoPrim;

    //Funcao
    if(sInfo.tipo_identificador == TID_FUNC){
      if(arvore->nFilhosMax != 2){
        printf("Erro (Linha %d): Identificador \"%s\" deve ser usado como funcao\n", arvore->valor_lexico.line_number, arvore->valor_lexico.valTokStr);
        exit(ERR_FUNCTION);
      }

      int nArgsArvore = 0;
      NODO_ARVORE* contador = arvore->filhos[0];
      while(contador != NULL){
        nArgsArvore++;
        contador = contador->filhos[contador->nFilhosMax - 1];
      }

      if(nArgsArvore < sInfo.nArgs){
        printf("Erro (Linha %d): Sem argumentos suficientes para a funcao \"%s\"\n", arvore->valor_lexico.line_number, arvore->valor_lexico.valTokStr);
        exit(ERR_MISSING_ARGS);
      }

      if(nArgsArvore > sInfo.nArgs){
        printf("Erro (Linha %d): Excesso de argumentos para a funcao \"%s\"\n", arvore->valor_lexico.line_number, arvore->valor_lexico.valTokStr);
        exit(ERR_EXCESS_ARGS);
      }

      ARG_LIST *cursor;
      contador = arvore->filhos[0];
      cursor = sInfo.argList;
      for(int i = 0; i < nArgsArvore; i++){
        if(contador->tipo != cursor->tipoArg.tipoPrim){
          printf("Erro (Linha %d): Argumentos de tipos diferentes para a funcao \"%s\"\n", arvore->valor_lexico.line_number, arvore->valor_lexico.valTokStr);
          exit(ERR_WRONG_TYPE_ARGS);
        }
        cursor = cursor->prox;
        contador = contador->filhos[contador->nFilhosMax - 1];
      }
    }

    return;
  }

  int isBinExp = 0, isUnExp = 0;
  const char binExps[14][10] = {"+", "-", "*", "/", "%", "|", "&", "^", "<", ">", "<=", ">=", "==", "!="};
  const char unExps[7][10] = {"+", "-", "!", "&", "*", "?", "#"};

  for(int i = 0; i < 14; i++)
    if(!strcmp(arvore->valor_lexico.valTokStr, binExps[i]))
      isBinExp = 1;

  if(isBinExp && !(arvore->nFilhosMax == 3))
    isBinExp = 0;

  for(int i = 0; i < 7; i++)
    if(!strcmp(arvore->valor_lexico.valTokStr, unExps[i]) && !isBinExp)
      isUnExp = 1;

  if(isBinExp){
    int tipo1 = arvore->filhos[0]->tipo;
    int tipo2 = arvore->filhos[1]->tipo;

    if(tipo1 == TL_INT && tipo2 == TL_INT)
      arvore->tipo = TL_INT;
    else if(tipo1 == TL_FLOAT && tipo2 == TL_FLOAT)
      arvore->tipo = TL_FLOAT;
    else if(tipo1 == TL_BOOL && tipo2 == TL_BOOL)
      arvore->tipo = TL_BOOL;
    else if((tipo1 == TL_FLOAT && tipo2 == TL_INT) || (tipo1 == TL_INT && tipo2 == TL_FLOAT))
      arvore->tipo = TL_FLOAT;
    else if((tipo1 == TL_BOOL && tipo2 == TL_INT) || (tipo1 == TL_INT && tipo2 == TL_BOOL))
      arvore->tipo = TL_INT;
    else if((tipo1 == TL_BOOL && tipo2 == TL_FLOAT) || (tipo1 == TL_FLOAT && tipo2 == TL_BOOL))
      arvore->tipo = TL_FLOAT;
    else if(tipo1 == TL_CHAR && tipo2 == TL_CHAR)
      arvore->tipo = TL_CHAR;
    else if(tipo1 == TL_STRING && tipo2 == TL_STRING)
      arvore->tipo = TL_STRING;
    else{
      printf("Erro (Linha %d): Operacao %s aplicada em operandos de tipos incompativeis", arvore->valor_lexico.line_number, arvore->valor_lexico.valTokStr);
      if(tipo1 == TL_CHAR || tipo2 == TL_CHAR)
        exit(ERR_CHAR_TO_X);
      if(tipo1 == TL_STRING || tipo2 == TL_STRING)
        exit(ERR_STRING_TO_X);
    }

    //Excessoes
    if(!strcmp(arvore->valor_lexico.valTokStr, ">") || !strcmp(arvore->valor_lexico.valTokStr, "<")
    || !strcmp(arvore->valor_lexico.valTokStr, "<=") || !strcmp(arvore->valor_lexico.valTokStr, ">=")
    || !strcmp(arvore->valor_lexico.valTokStr, "==") || !strcmp(arvore->valor_lexico.valTokStr, "!=")){
      arvore->tipo = TL_BOOL;
    }

    return;
  }

  if(isUnExp){
    int tipo1 = arvore->filhos[0]->tipo;
    arvore->tipo = tipo1;
    return;
  }

  //Operador ternario
  if(!strcmp(arvore->valor_lexico.valTokStr, "?:")){
    if(arvore->filhos[0]->tipo != TL_BOOL){
      printf("Erro (Linha %d): Primeiro operando do operador ternario deve ser BOOL\n", arvore->valor_lexico.line_number);
      exit(ERR_WRONG_TYPE);
    }
    if(arvore->filhos[1]->tipo != arvore->filhos[2]->tipo){
      printf("Erro (Linha %d): Expressoes do operador ternario devem ter o mesmo tipo\n", arvore->valor_lexico.line_number);
      exit(ERR_WRONG_TYPE);
    }
    arvore->tipo = arvore->filhos[1]->tipo;
    return;
  }

  //If e While
  if(!strcmp(arvore->valor_lexico.valTokStr, "if") || !strcmp(arvore->valor_lexico.valTokStr, "while")){
    if(arvore->filhos[0]->tipo != TL_BOOL){
      printf("Erro (Linha %d): Teste do \"%s\" deve ser do tipo BOOL\n", arvore->valor_lexico.line_number, arvore->valor_lexico.valTokStr);
      exit(ERR_WRONG_TYPE);
    }

    arvore->tipo = TL_NONE;
    return;
  }

  //For
  if(!strcmp(arvore->valor_lexico.valTokStr, "for")){
    if(arvore->filhos[1]->tipo != TL_BOOL){
      printf("Erro (Linha %d): Teste do \"%s\" deve ser do tipo BOOL\n", arvore->valor_lexico.line_number, arvore->valor_lexico.valTokStr);
      exit(ERR_WRONG_TYPE);
    }

    arvore->tipo = TL_NONE;
    return;
  }

  //Shifts
  if(!strcmp(arvore->valor_lexico.valTokStr, "<<") || !strcmp(arvore->valor_lexico.valTokStr, ">>")){
    if(arvore->filhos[1]->tipo != TL_INT){
      printf("Erro (Linha %d): Deslocamento do shift deve ser inteiro\n", arvore->valor_lexico.line_number);
      exit(ERR_WRONG_TYPE);
    }

    arvore->tipo = TL_NONE;
    return;
  }

  //Vetores
  if(!strcmp(arvore->valor_lexico.valTokStr, "[]")){
    consulta_tabela(tabela, arvore->filhos[0]->valor_lexico.valTokStr, &sInfo);
    if(sInfo.tipo_identificador == TID_VAR){
      printf("Erro (Linha %d): Identificador \"%s\" deve ser usado como variavel\n", arvore->valor_lexico.line_number, arvore->filhos[0]->valor_lexico.valTokStr);
      exit(ERR_VARIABLE);
    }
    if(sInfo.tipo_identificador == TID_FUNC){
      printf("Erro (Linha %d): Identificador \"%s\" deve ser usado como funcao\n", arvore->valor_lexico.line_number, arvore->filhos[0]->valor_lexico.valTokStr);
      exit(ERR_FUNCTION);
    }
    if(arvore->filhos[1]->tipo != TL_INT){
      printf("Erro (Linha %d): O tipo do indice de um vetor deve ser int\n", arvore->valor_lexico.line_number);
      exit(ERR_WRONG_TYPE);
    }

    arvore->tipo = arvore->filhos[0]->tipo;
    return;
  }

  //Atribuicoes
  if(!strcmp(arvore->valor_lexico.valTokStr, "=")){
    int tipo1 = arvore->filhos[0]->tipo;
    int tipo2 = arvore->filhos[1]->tipo;

    char tipo1S[20], tipo2S[20];
    switch (tipo1) {
      case TL_FLOAT:
        sprintf(tipo1S, "float");
        break;
      case TL_INT:
        sprintf(tipo1S, "int");
        break;
      case TL_BOOL:
        sprintf(tipo1S, "bool");
        break;
    }
    switch (tipo2) {
      case TL_FLOAT:
        sprintf(tipo2S, "float");
        break;
      case TL_INT:
        sprintf(tipo2S, "int");
        break;
      case TL_BOOL:
        sprintf(tipo2S, "bool");
        break;
    }

    if(tipo1 != tipo2){
        if(tipo1 == TL_FLOAT && (tipo2 == TL_BOOL || tipo2 == TL_INT))
          arvore->tipo = TL_FLOAT;
        else if(tipo1 == TL_INT && (tipo2 == TL_BOOL || tipo2 == TL_FLOAT))
          arvore->tipo = TL_INT;
        else if(tipo1 == TL_BOOL && (tipo2 == TL_FLOAT || tipo2 == TL_INT))
          arvore->tipo = TL_BOOL;
        else{
          printf("Erro (Linha %d): Atribuicao de tipos incompativeis\n", arvore->valor_lexico.line_number);
          exit(ERR_WRONG_TYPE);
        }
        printf("Warning (Linha %d): Conversao implicita de %s para %s\n", arvore->valor_lexico.line_number, tipo2S, tipo1S);
    }
    else
      arvore->tipo = tipo1;

    return;
  }

  //Comando break e continue
  if(!strcmp(arvore->valor_lexico.valTokStr, "break") || !strcmp(arvore->valor_lexico.valTokStr, "continue")){

    arvore->tipo = TL_NONE;
  }

  //Comando return
  if(!strcmp(arvore->valor_lexico.valTokStr, "return")){
    if(arvore->filhos[0]->tipo != getTipoUltimaFuncao(tabela)){
      printf("Erro (Linha %d): Retorno de tipo incorreto\n", arvore->valor_lexico.line_number);
      exit(ERR_WRONG_PAR_RETURN);
    }

    arvore->tipo = TL_NONE;
  }

}

void format(int spaces) {
  int lines = 2, columns = 3;

  for(int j=0; j<lines; j++) {
    printf("\n");
    for(int k=0; k<columns*spaces;k++) {
      printf(" ");
    }
    printf("|");
  }

  for(int k=0; k<columns-1; k++) {
    printf("-");
  }
}

void printArvore(NODO_ARVORE* arvore, int spaces){

  if(arvore == NULL){
    return;
  }

  if(spaces != 0)
    format(spaces);

  if(arvore->valor_lexico.tipo_token == TT_LIT){
    switch(arvore->valor_lexico.tipo_literal){
      case TL_INT:
        printf("(LitInt %d)", arvore->valor_lexico.valTokInt);
        break;
      case TL_CHAR:
        printf("(LitChar %c)", arvore->valor_lexico.valTokChar);
        break;
      case TL_FLOAT:
        printf("(LitFloat %f)", arvore->valor_lexico.valTokFloat);
        break;
      case TL_BOOL:
        printf("(LitBool %d)", arvore->valor_lexico.valTokBool);
        break;
      case TL_STRING:
      printf("(LitStr %s)", arvore->valor_lexico.valTokStr);
      break;
    }
  }
  else{
    printf("(%s) - Tipo: ", arvore->valor_lexico.valTokStr);
    switch (arvore->tipo) {
      case TL_INT:
        printf("int");
        break;
      case TL_FLOAT:
        printf("float");
        break;
      case TL_BOOL:
        printf("bool");
        break;
      case TL_STRING:
        printf("string");
        break;
      case TL_CHAR:
        printf("char");
        break;
      case TL_NONE:
        printf("none / void");
        break;
      case TL_UNKNOWN:
        printf("?");
        break;
    }
  }

  for(int i = 0; i < arvore->nFilhosMax; i++){
    if(arvore->filhos[i] != NULL) {
      printArvore(arvore->filhos[i], spaces+1);
    }
  }
}

void libera_arvore(NODO_ARVORE* arvore){
  if(arvore == NULL)
    return;

  for(int i = 0; i < arvore->nFilhosMax; i++){
    libera_arvore(arvore->filhos[i]);
  }

  if(arvore->nFilhosMax > 0){
    free(arvore->filhos);
    arvore->filhos = NULL;
  }

  switch (arvore->valor_lexico.tipo_literal) {
    case TL_NONE:
    case TL_STRING:
      free(arvore->valor_lexico.valTokStr);
      break;
    default:
      break;
  }

  free(arvore);
}

void exporta_arvore(NODO_ARVORE* arvore, FILE* fp){
  if(arvore == NULL){
    return;
  }

  for(int i = 0; i < arvore->nFilhosMax; i++){
    if(arvore->filhos[i] != NULL){
      fprintf(fp, "%p, %p\n", arvore, arvore->filhos[i]);
      exporta_arvore(arvore->filhos[i], fp);
    }
  }
}
