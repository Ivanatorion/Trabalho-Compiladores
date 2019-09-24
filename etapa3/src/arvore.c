#include "../include/arvore.h"

#include <stdio.h>
#include <stdlib.h>

NODO_ARVORE* createNode(struct valLex valor, int nFilhos){
  NODO_ARVORE *nodo = malloc(sizeof(NODO_ARVORE));

  if(nFilhos > 0)
    nodo->filhos = malloc(sizeof(NODO_ARVORE*) * nFilhos);
  else
    nodo->filhos = NULL;

  nodo->valor_lexico = valor;
  return nodo;
}
