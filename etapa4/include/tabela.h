#ifndef TABELA_S
#define TABELA_S

#include "defines.h"

#define INIT_MAX_ENTRADAS 8

typedef struct argl{
  struct argl *prox;

  char *arg;
  int tipoArg;
} ARG_LIST;

typedef struct simbolo_info{
  int linha;
  int natureza;
  int tipo;
  int tamanho;

  ARG_LIST* argList;
  int nArgs;

  union{
    char *idName;
    int litIntVal;
    int litBoolVal;
    float litFloatVal;
  };

} S_INFO;

typedef struct tabela_s{
  struct tabela_s *prox;
  struct tabela_s *ant;

  int maxEntradas;
  int nEntradas;

  S_INFO **entradas;
} T_SIMBOLO;

T_SIMBOLO* make_tabela();

void free_tabela(T_SIMBOLO* tabela);

//Retorna codigos de erro
int insere_tabela(T_SIMBOLO* tabela, S_INFO info);

//Retorna codigos de erro, armazena em "info" o valor armazenado na tabela
int consulta_tabela(T_SIMBOLO* tabela, char* chave, S_INFO* info);

void print_tabela(T_SIMBOLO* tabela);

#endif
