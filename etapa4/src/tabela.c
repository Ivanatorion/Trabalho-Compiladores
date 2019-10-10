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

    for(int i = 0; i < tabela->maxEntradas; i++)
      entradasNovas[i] = NULL;

    //Re-hash
    for(int i = 0; i < tabela->nEntradas; i++){
      int posicao = hash(tabela->entradas[i]->idName) % tabela->maxEntradas;

      while(entradasNovas[posicao] != NULL){
        posicao++;
        if(posicao == tabela->maxEntradas)
          posicao = 0;
      }

      entradasNovas[posicao] = tabela->entradas[i];
    }

    free(tabela->entradas);
    tabela->entradas = entradasNovas;
  }

  int posicao = hash(info.idName) % tabela->maxEntradas;

  while(tabela->entradas[posicao] != NULL){

    if(!strcmp(tabela->entradas[posicao]->idName, info.idName))
      return ERR_DECLARED;

    posicao++;
    if(posicao == tabela->maxEntradas)
      posicao = 0;
  }

  S_INFO* entrada = (S_INFO*) malloc(sizeof(S_INFO));
  *entrada = info;

  if(entrada->natureza == NATUREZA_IDENTIFICADOR)
    entrada->idName = strdup(entrada->idName);

  tabela->entradas[posicao] = entrada;

  tabela->nEntradas++;

  return 0;
}

int consulta_tabela(T_SIMBOLO* tabela, char* chave, S_INFO* info){
  if(DEBUG_MODE)
    printf("\033[0;32m\nProcurando por: %s\n\033[0m", chave);

  int posicao = hash(chave) % tabela->maxEntradas;

  int verificados = 0;
  while(tabela->entradas[posicao] != NULL && verificados != tabela->nEntradas){
    verificados++;

    if(DEBUG_MODE)
      printf("\033[0;32m%d) %s\n\033[0m", verificados, tabela->entradas[posicao]->idName);

    if(!strcmp(chave, tabela->entradas[posicao]->idName)){
      *info = *(tabela->entradas[posicao]);

      if(DEBUG_MODE)
        printf("\033[0;32mEncontrou %s depois de %d verificacoes\n\n\033[0m", chave, verificados);

      return 0;
    }

    posicao++;
    if(posicao == tabela->maxEntradas)
      posicao = 0;
  }

  if(DEBUG_MODE)
    printf("\033[0;32mNao encontrou %s. Verificados %d.\n\n\033[0m", chave, verificados+1);

  return ERR_UNDECLARED;
}

void free_tabela(T_SIMBOLO* tabela){
  if(tabela->prox != NULL)
    tabela->prox->ant = tabela->ant;

  if(tabela->ant != NULL)
    tabela->ant->prox = tabela->prox;

  for(int i = 0; i < tabela->maxEntradas; i++){
    if(tabela->entradas[i] != NULL){
      if(tabela->entradas[i]->natureza == NATUREZA_IDENTIFICADOR)
        free(tabela->entradas[i]->idName);

      free(tabela->entradas[i]);
    }
  }

  free(tabela->entradas);
  free(tabela);
}

void free_tabela_recursive(T_SIMBOLO* tabela){
  if(tabela->prox != NULL)
    tabela->prox->ant = tabela->ant;

  if(tabela->ant != NULL)
    tabela->ant->prox = tabela->prox;

  for(int i = 0; i < tabela->maxEntradas; i++){
    if(tabela->entradas[i] != NULL){
      if(tabela->entradas[i]->natureza == NATUREZA_IDENTIFICADOR)
        free(tabela->entradas[i]->idName);

      free(tabela->entradas[i]);
    }
  }

  free(tabela->entradas);

  if(tabela->ant != NULL)
    free_tabela_recursive(tabela->ant);
  else if(tabela->prox != NULL)
    free_tabela_recursive(tabela->prox);

  free(tabela);
}

void print_tabela(T_SIMBOLO* tabela){
  if(!DEBUG_MODE)
    return;

  for(int i = 0; i < tabela->maxEntradas; i++)
    if(tabela->entradas[i] == NULL){
      printf("%d) ", i);
      printf("\033[0;31mNULL\n\033[0m");
    }
    else{
      printf("%d) %s ( ", i, tabela->entradas[i]->idName);
      switch (tabela->entradas[i]->natureza) {
        case NATUREZA_IDENTIFICADOR:
          printf("Identificador - ");
          switch (tabela->entradas[i]->tipo_identificador) {
            case TID_VAR:
              printf("Variavel | ");
              break;
            case TID_FUNC:
              printf("Funcao() | ");
              break;
            case TID_VET:
              printf("Vetor | ");
              break;
          }
          break;
      }
      printf("Linha: %d | ", tabela->entradas[i]->linha);
      printf("Tipo: ");
      switch (tabela->entradas[i]->tipo.tipoPrim) {
        case TL_INT:
          printf("int");
          break;
        case TL_FLOAT:
          printf("float");
          break;
        case TL_BOOL:
          printf("bool");
          break;
        case TL_CHAR:
          printf("char");
          break;
        case TL_STRING:
          printf("string");
          break;
      }

      printf(")\n");
    }
}
