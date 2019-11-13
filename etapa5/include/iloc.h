#ifndef ILOC_HEADER_H
#define ILOC_HEADER_H

#include "../include/arvore.h"
#include "../include/tabela.h"

//Gera o arquivo "saida.iloc"
void genSaidaIloc(NODO_ARVORE* arvore, T_SIMBOLO* tabela);

void genNodeCode(NODO_ARVORE* nodo, T_SIMBOLO* tabela);

#endif
