#include "../include/labeltable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int isPrimeLT(int n){
  for(int i = 2; i < n/2; i++)
    if(n%i == 0)
      return 0;

  return 1;
}

unsigned long hashLT(char* str){
  //djb2
  //http://www.cse.yorku.ca/~oz/hash.html

  unsigned long hash = 5381;
  int c;
  while ((c = *str++))
      hash = ((hash << 5) + hash) + c;
  return hash;
}

LABEL_TABLE* make_label_table(){
  LABEL_TABLE* novaTabela = (LABEL_TABLE*) malloc(sizeof(LABEL_TABLE));

  novaTabela->nEntradasMax = INITIAL_ENTRIES;
  novaTabela->nEntradas = 0;

  novaTabela->entradas = (LABEL_TABLE_ENTRY**) malloc(sizeof(LABEL_TABLE_ENTRY*) * INITIAL_ENTRIES);

  for(int i = 0; i < INITIAL_ENTRIES; i++)
    novaTabela->entradas[i] = NULL;

  return novaTabela;
}

int insere_label_table(LABEL_TABLE* label_table, char* function_name, char* function_label, int accDesloc){

  if(label_table->nEntradas == label_table->nEntradasMax){
    //Resize
    label_table->nEntradasMax = label_table->nEntradasMax * 2;
    while(!isPrimeLT(label_table->nEntradasMax))
      label_table->nEntradasMax++;

    LABEL_TABLE_ENTRY** entradasNovas = (LABEL_TABLE_ENTRY**) malloc(sizeof(LABEL_TABLE_ENTRY*) * label_table->nEntradasMax);

    for(int i = 0; i < label_table->nEntradasMax; i++)
      entradasNovas[i] = NULL;

    //Re-hash
    for(int i = 0; i < label_table->nEntradas; i++){
      int posicao = hashLT(label_table->entradas[i]->key) % label_table->nEntradasMax;

      while(entradasNovas[posicao] != NULL){
        posicao++;
        if(posicao == label_table->nEntradasMax)
          posicao = 0;
      }

      entradasNovas[posicao] = label_table->entradas[i];
    }

    free(label_table->entradas);
    label_table->entradas = entradasNovas;
  }

  int posicao = hashLT(function_name) % label_table->nEntradasMax;

  while(label_table->entradas[posicao] != NULL){
    if(!strcmp(label_table->entradas[posicao]->key, function_name)){
      label_table->entradas[posicao]->accDesloc = accDesloc;
      return -1;
    }
    posicao++;
    if(posicao == label_table->nEntradasMax)
      posicao = 0;
  }

  LABEL_TABLE_ENTRY* entrada = malloc(sizeof(LABEL_TABLE_ENTRY));

  entrada->key = strdup(function_name);
  entrada->value = strdup(function_label);
  entrada->accDesloc = accDesloc;

  label_table->entradas[posicao] = entrada;

  label_table->nEntradas++;

  return 0;
}

LABEL_TABLE_ENTRY* consulta_label_table(LABEL_TABLE* label_table, char* chave){
  int posicao = hashLT(chave) % label_table->nEntradasMax;
  int verificados = 0;

  while(label_table->entradas[posicao] != NULL && verificados != label_table->nEntradas){
    verificados++;

    if(!strcmp(chave, label_table->entradas[posicao]->key))
      return label_table->entradas[posicao];

    posicao++;
    if(posicao == label_table->nEntradasMax)
      posicao = 0;
  }

  return NULL;
}


void free_label_table(LABEL_TABLE* label_table){
  if(label_table == NULL)
    return;

  for(int i = 0; i < label_table->nEntradasMax; i++){
    if(label_table->entradas[i] != NULL){
      free(label_table->entradas[i]->key);
      free(label_table->entradas[i]->value);
      free(label_table->entradas[i]);
    }
  }

  free(label_table->entradas);
  free(label_table);
}

void print_label_table(LABEL_TABLE* label_table){
  if(label_table == NULL)
    return;

  for(int i = 0; i < label_table->nEntradasMax; i++){
    if(label_table->entradas[i] == NULL){
      printf("%d) ", i);
      printf("\033[0;31mNULL\n\033[0m");
    }
    else{
      printf("%d) %s -> %s (%d Frame size)\n", i, label_table->entradas[i]->key, label_table->entradas[i]->value, label_table->entradas[i]->accDesloc);
    }
  }
  printf("\n");
}
