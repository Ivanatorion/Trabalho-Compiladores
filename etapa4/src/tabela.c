#include "../include/tabela.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long hash(char* str){
  //djb2
  //http://www.cse.yorku.ca/~oz/hash.html

  unsigned long hash = 5381;
  int c;
  while (c = *str++)
      hash = ((hash << 5) + hash) + c;
  return hash;
}

T_SIMBOLO* make_tabela(){
  T_SIMBOLO* novaTabela = (T_SIMBOLO*) malloc(sizeof(T_SIMBOLO));

  novaTabela->prox = NULL;
  novaTabela->ant = NULL;

  novaTabela->maxEntradas = INIT_MAX_ENTRADAS;
  novaTabela->nEntradas = 0;

  novaTabela->entradas = (S_INFO**) malloc(sizeof(S_INFO*) * INIT_MAX_ENTRADAS);

  for(int i = 0; i < INIT_MAX_ENTRADAS; i++)
    novaTabela->entradas[i] = NULL;

  return novaTabela;
}

int insere_tabela(T_SIMBOLO* tabela, S_INFO info){
  if(tabela->nEntradas == tabela->maxEntradas){
    //Resize
    tabela->maxEntradas = tabela->maxEntradas * 2;
    S_INFO **entradasNovas = (S_INFO**) malloc(sizeof(S_INFO*) * tabela->maxEntradas);

    for(int i = 0; i < tabela->nEntradas; i++)
      entradasNovas[i] = tabela->entradas[i];

    for(int i = tabela->nEntradas; i < tabela->maxEntradas; i++)
      entradasNovas[i] = NULL;

    free(tabela->entradas);
    tabela->entradas = entradasNovas;
  }

  int posicao = hash(info.valor.valTokStr) % tabela->maxEntradas;

  while(tabela->entradas[posicao] != NULL){

    if(!strcmp(tabela->entradas[posicao]->valor.valTokStr, info.valor.valTokStr))
      return ERR_DECLARED;

    posicao++;
    if(posicao == tabela->maxEntradas)
      posicao = 0;
  }

  S_INFO* entrada = (S_INFO*) malloc(sizeof(S_INFO));
  *entrada = info;
  tabela->entradas[posicao] = entrada;

  tabela->nEntradas++;

  return 0;
}

void free_tabela(T_SIMBOLO* tabela){
  if(tabela->prox != NULL)
    tabela->prox->ant = tabela->ant;

  if(tabela->ant != NULL)
    tabela->ant->prox = tabela->prox;

  for(int i = 0; i < tabela->maxEntradas; i++)
    if(tabela->entradas[i] != NULL)
      free(tabela->entradas[i]);

  free(tabela->entradas);
  free(tabela);
}

void print_tabela(T_SIMBOLO* tabela){
  for(int i = 0; i < tabela->maxEntradas; i++)
    if(tabela->entradas[i] == NULL)
      printf("%d) NULL\n", i);
    else
      printf("%d) %s\n", i, tabela->entradas[i]->valor.valTokStr);
}
