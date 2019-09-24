#ifndef ARVORE_H
#define ARVORE_H

#include "../parser.tab.h"

extern void *arvore;

NODO_ARVORE* createNode(struct valLex valor, int nFilhos);

#endif
