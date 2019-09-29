#include "../include/arvore.h"

#include <stdio.h>
#include <stdlib.h>

NODO_ARVORE* createNode(struct valLex valor, int nFilhos){
  NODO_ARVORE *nodo = malloc(sizeof(NODO_ARVORE));
  nodo->nFilhos = nFilhos;

  if(nFilhos > 0){
    nodo->filhos = malloc(sizeof(NODO_ARVORE*) * nFilhos);
    for(int i = 0; i < nFilhos; i++)
      nodo->filhos[i] = NULL;
  }
  else
    nodo->filhos = NULL;

  nodo->valor_lexico = valor;
  return nodo;
}

void addFilho(NODO_ARVORE* pai, NODO_ARVORE* filho){
  int i = 0;
  while(pai->filhos[i] != NULL)
    i++;

  pai->filhos[i] = filho;
}

void printArvore(NODO_ARVORE* arvore){
  if(arvore == NULL){
    printf("Null Leaf: %p\n", arvore);
    return;
  }

  if(arvore->nFilhos == 0){
    printf("Leaf: %p ", arvore);
  }
  else{
    printf("Node: %p ", arvore);
  }
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
    printf("(%s)", arvore->valor_lexico.valTokStr);
  }
  printf("\n");

  for(int i = 0; i < arvore->nFilhos; i++){
    printf("%p: %p\n", arvore, arvore->filhos[i]);
    printArvore(arvore->filhos[i]);
  }
}

void libera_arvore(NODO_ARVORE* arvore){
  if(arvore == NULL)
    return;

  for(int i = 0; i < arvore->nFilhos; i++){
    libera_arvore(arvore->filhos[i]);
  }

  if(arvore->nFilhos > 0){
    free(arvore->filhos);
    arvore->filhos = NULL;
  }

  switch (arvore->valor_lexico.tipo_literal) {
    case TL_NONE:
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

  for(int i = 0; i < arvore->nFilhos; i++){
    fprintf(fp, "%p, %p\n", arvore, arvore->filhos[i]);
    exporta_arvore(arvore->filhos[i], fp);
  }
}
