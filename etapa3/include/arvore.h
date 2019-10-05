#ifndef ARVORE_H
#define ARVORE_H

#include "../parser.tab.h"
#include "defines.h"
#include <stdio.h>

extern void *arvore;

NODO_ARVORE* createNode(struct valLex valor, int nFilhosMax);

void addFilho(NODO_ARVORE* pai, NODO_ARVORE* filho);

void libera_arvore(NODO_ARVORE* arvore);
void exporta_arvore(NODO_ARVORE* arvore, FILE* fp);

void printArvore(NODO_ARVORE* arvore, int spaces);

#endif
