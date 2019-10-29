#ifndef TABELA_S
#define TABELA_S

#include "../parser.tab.h"
#include "defines.h"

#define INIT_MAX_ENTRADAS 11

typedef struct simbolo_info{
  int linha;
  int natureza;
  TIPO_COMPOSTO tipo;
  int tamanho;

  int tipoIdentificador; //Variavel, vetor, funcao

  ARG_LIST* argList;
  int nArgs;

  char *idName;

  int varDesloc;
  int isVarGlob;

} S_INFO;

typedef struct tabela_s{
  struct tabela_s *prox;
  struct tabela_s *ant;

  TIPO_COMPOSTO tipoFuncaoTabela; //Tipo da funcao a qual essa tabela pertence. Caso nao pertenca a uma funcao, deve ser TL_NONE. Usado para decidir se o "return" esta correto.

  int maxEntradas;
  int nEntradas;

  S_INFO **entradas;

  int accDesloc;
} T_SIMBOLO;

T_SIMBOLO* make_tabela();

void pushEscopo(T_SIMBOLO* tabela, ARG_LIST* iniciais, TIPO_COMPOSTO tipoFuncaoT);

void popEscopo(T_SIMBOLO* tabela);

void free_tabela(T_SIMBOLO* tabela);

void free_tabela_recursive(T_SIMBOLO* tabela);

//Retorna o tipo da ultima funcao empilhada
//OBS: Essa funcao assume que exite uma tabela na pilha com simbolo != TL_NONE!
//     Como foi feita para verificar o tipo do "return", isso sempre deve ser verdade
int getTipoUltimaFuncao(T_SIMBOLO* tabela);

//Retorna codigos de erro
int insere_tabela(T_SIMBOLO* tabela, S_INFO info);

//Retorna codigos de erro, armazena em "info" o valor armazenado na tabela
int consulta_tabela(T_SIMBOLO* tabela, char* chave, S_INFO* info);

void print_tabela(T_SIMBOLO* tabela);

#endif
