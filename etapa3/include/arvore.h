#ifndef ARVORE_H
#define ARVORE_H

#include "../parser.tab.h"
#include "defines.h"

extern void *arvore;

NODO_ARVORE* createNode(struct valLex valor, int nFilhos);
void addFilho(NODO_ARVORE* pai, NODO_ARVORE* filho);

void printArvore(NODO_ARVORE* arvore);

#endif
