#ifndef TABELA_LABELS_S
#define TABELA_LABELS_S

#include "defines.h"

#define INITIAL_ENTRIES 5

typedef struct lbtb_entry{
  char *key;
  char *value;
} LABEL_TABLE_ENTRY;

typedef struct lbtb{
  int nEntradas;
  int nEntradasMax;

  LABEL_TABLE_ENTRY **entradas;
} LABEL_TABLE;

LABEL_TABLE* make_label_table();

void free_label_table(LABEL_TABLE* label_table);

//Retorna codigos de erro
int insere_label_table(LABEL_TABLE* label_table, char* function_name, char* function_label);

//Retorna NULL se nao achar
char* consulta_label_table(LABEL_TABLE* label_table, char* chave);

void print_label_table(LABEL_TABLE* label_table);

#endif
