#include "../include/tabela.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int isPrime(int n){
  for(int i = 2; i < n/2; i++)
    if(n%i == 0)
      return 0;

  return 1;
}

unsigned long hash(char* str){
  //djb2
  //http://www.cse.yorku.ca/~oz/hash.html

  unsigned long hash = 5381;
  int c;
  while ((c = *str++))
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

  novaTabela->accDesloc = 0;

  return novaTabela;
}

void pushEscopo(T_SIMBOLO* tabela, ARG_LIST* iniciais, TIPO_COMPOSTO tipoFuncaoT){
  while(tabela->prox != NULL)
    tabela = tabela->prox;

  tabela->prox = make_tabela();
  tabela->prox->ant = tabela;

  tabela->prox->tipoFuncaoTabela = tipoFuncaoT;

  if(tipoFuncaoT.tipoPrim == TL_NONE){
    tabela->prox->accDesloc = tabela->accDesloc;
  }
  else{
    tabela->prox->accDesloc = STACK_FRAME_TAM_FIX;
  }

  //Insere as entradas iniciais
  tabela = tabela->prox;
  S_INFO sInfo;
  sInfo.natureza = NATUREZA_IDENTIFICADOR;
  sInfo.tipoIdentificador = TID_VAR;
  sInfo.argList = NULL;

  while(iniciais != NULL){
    sInfo.idName = iniciais->arg;
    sInfo.tipo = iniciais->tipoArg;
    sInfo.linha = iniciais->linhaArg;
    insere_tabela(tabela, sInfo, NULL);
    iniciais = iniciais->prox;
  }
}

int popEscopo(T_SIMBOLO* tabela){
  while(tabela->prox != NULL)
    tabela = tabela->prox;

  if(tabela->tipoFuncaoTabela.tipoPrim == TL_NONE){
    tabela->ant->accDesloc = tabela->accDesloc;
  }

  int retorno = tabela->accDesloc;

  free_tabela(tabela);

  return retorno;
}

int getTipoUltimaFuncao(T_SIMBOLO* tabela){
  while(tabela->prox != NULL)
    tabela = tabela->prox;

  while(tabela->tipoFuncaoTabela.tipoPrim == TL_NONE)
    tabela = tabela->ant;

  return tabela->tipoFuncaoTabela.tipoPrim;
}

int insere_tabela(T_SIMBOLO* tabela, S_INFO info, NODO_ARVORE* dimensions){
  //Se for identificador, insere no escopo mais profundo
  if(info.natureza == NATUREZA_IDENTIFICADOR){
    while(tabela->prox != NULL)
      tabela = tabela->prox;
  }
  //Se for literal, insere no escopo global
  else{
    while(tabela->ant != NULL)
      tabela = tabela->ant;
  }

  //Dimensoes de arranjos
  info.nDims = 0;
  info.dimList = NULL;
  NODO_ARVORE* auxNAP = dimensions;
  NODO_ARVORE* auxNAP2;
  DIM_LIST* auxDL = NULL;
  while(dimensions != NULL){
    info.nDims++;
    info.dimList = malloc(sizeof(DIM_LIST));
    info.dimList->prox = auxDL;
    info.dimList->dim = dimensions->filhos[1]->valor_lexico.valTokInt;
    dimensions = dimensions->filhos[0];
    auxDL = info.dimList;
  }
  while(auxNAP != NULL){
    free(auxNAP->filhos[1]);
    auxNAP2 = auxNAP->filhos[0];
    free(auxNAP->filhos);
    free(auxNAP);
    auxNAP = auxNAP2;
  }

  if(tabela->nEntradas == tabela->maxEntradas){
    //Resize
    tabela->maxEntradas = tabela->maxEntradas * 2;
    while(!isPrime(tabela->maxEntradas))
      tabela->maxEntradas++;

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

    if(!strcmp(tabela->entradas[posicao]->idName, info.idName)){
        if(info.natureza == NATUREZA_IDENTIFICADOR){
          printf("Erro (Linha %d): Redeclaracao do identificador \"%s\". Declarado anteriormente na Linha %d\n", info.linha, info.idName, tabela->entradas[posicao]->linha);
          exit(ERR_DECLARED);
        }

        //Literal
        tabela->entradas[posicao]->linha = info.linha;
        tabela->entradas[posicao]->tipo = info.tipo;
        tabela->entradas[posicao]->natureza = info.natureza;
        //Atualiza o tamanho
        switch(info.tipo.tipoPrim){
          case TL_CHAR:
            info.tamanho = 1;
            break;
          case TL_STRING:
            info.tamanho = strlen(info.idName) + 1;
            break;
          case TL_INT:
            info.tamanho = 4;
            break;
          case TL_FLOAT:
            info.tamanho = 8;
            break;
          case TL_BOOL:
            info.tamanho = 1;
            break;
        }
        return 0;
    }

    posicao++;
    if(posicao == tabela->maxEntradas)
      posicao = 0;
  }

  //Atualiza o tamanho e deslocamento da Variavel
  info.varDesloc = tabela->accDesloc;
  switch(info.tipo.tipoPrim){
    case TL_CHAR:
      info.tamanho = 1;
      break;
    case TL_STRING:
      info.tamanho = 0; //??
      break;
    case TL_INT:
      info.tamanho = 4;
      break;
    case TL_FLOAT:
      info.tamanho = 8;
      break;
    case TL_BOOL:
      info.tamanho = 4;
      break;
  }

  auxDL = info.dimList;
  while(auxDL != NULL){
    info.tamanho = info.tamanho * auxDL->dim;
    auxDL = auxDL->prox;
  }
  if(info.tipoIdentificador != TID_FUNC && info.natureza == NATUREZA_IDENTIFICADOR)
    tabela->accDesloc = tabela->accDesloc + info.tamanho;

  S_INFO* entrada = (S_INFO*) malloc(sizeof(S_INFO));
  *entrada = info;

  //Atualiza nArgs
  ARG_LIST* auxPointer = entrada->argList;
  entrada->nArgs = 0;
  while(auxPointer != NULL){
    entrada->nArgs++;
    auxPointer = auxPointer->prox;
  }

  entrada->idName = strdup(entrada->idName);
  entrada->isVarGlob = (tabela->ant == NULL) ? 1 : 0;

  tabela->entradas[posicao] = entrada;

  tabela->nEntradas++;

  return 0;
}

int consulta_tabela(T_SIMBOLO* tabela, char* chave, S_INFO* info){
  while(tabela->prox != NULL)
    tabela = tabela->prox;

  if(DEBUG_MODE)
    printf("\033[0;32m\nProcurando por: %s\n\033[0m", chave);

  int posicao, verificados;
  while(tabela != NULL){
    posicao = hash(chave) % tabela->maxEntradas;

    verificados = 0;
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

    tabela = tabela->ant;
  }

  return ERR_UNDECLARED;
}

void free_tabela(T_SIMBOLO* tabela){
  if(tabela == NULL)
    return;

  if(tabela->prox != NULL)
    tabela->prox->ant = tabela->ant;

  if(tabela->ant != NULL)
    tabela->ant->prox = tabela->prox;

  for(int i = 0; i < tabela->maxEntradas; i++){
    if(tabela->entradas[i] != NULL){
      free(tabela->entradas[i]->idName);

      free(tabela->entradas[i]);
    }
  }

  free(tabela->entradas);
  free(tabela);
}

void free_tabela_recursive(T_SIMBOLO* tabela){
  if(tabela == NULL)
    return;

  if(tabela->prox != NULL)
    tabela->prox->ant = tabela->ant;

  if(tabela->ant != NULL)
    tabela->ant->prox = tabela->prox;

  for(int i = 0; i < tabela->maxEntradas; i++){
    if(tabela->entradas[i] != NULL){
      free(tabela->entradas[i]->idName);

      if(tabela->entradas[i]->argList != NULL){
        ARG_LIST* auxPointer = tabela->entradas[i]->argList->prox;
        while(auxPointer != NULL){
          free(tabela->entradas[i]->argList->arg);
          free(tabela->entradas[i]->argList);
          tabela->entradas[i]->argList = auxPointer;
          auxPointer = auxPointer->prox;
        }
        free(tabela->entradas[i]->argList->arg);
        free(tabela->entradas[i]->argList);
      }

      if(tabela->entradas[i]->dimList != NULL){
        DIM_LIST* auxPointer = tabela->entradas[i]->dimList->prox;
        while(auxPointer != NULL){
          free(tabela->entradas[i]->dimList);
          tabela->entradas[i]->dimList = auxPointer;
          auxPointer = auxPointer->prox;
        }
        free(tabela->entradas[i]->dimList);
      }

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
  if(!DEBUG_MODE || tabela == NULL)
    return;

  while(tabela->prox != NULL)
    tabela = tabela->prox;

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
          switch (tabela->entradas[i]->tipoIdentificador) {
            case TID_VAR:
              printf("Variavel | ");
              break;
            case TID_FUNC:
              printf("Funcao() %d Args | ", tabela->entradas[i]->nArgs);
              ARG_LIST* cursor = tabela->entradas[i]->argList;
              while(cursor != NULL){
                printf("Arg %s ", cursor->arg);

                printf("Tipo: ");
                switch (cursor->tipoArg.tipoPrim) {
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
                printf(" | ");
                cursor = cursor->prox;
              }

              break;
            case TID_VET:
              printf("Vetor %d Dims | ", tabela->entradas[i]->nDims);
              DIM_LIST* cursorD = tabela->entradas[i]->dimList;
              while(cursorD != NULL){
                printf("[%d] ", cursorD->dim);
                cursorD = cursorD->prox;
              }
              printf("| ");
              break;
          }
          break;
        case NATUREZA_LITERAL_INT:
          printf("Literal int | ");
          break;
        case NATUREZA_LITERAL_CHAR:
          printf("Literal char | ");
          break;
        case NATUREZA_LITERAL_FLOAT:
          printf("Literal float | ");
          break;
        case NATUREZA_LITERAL_BOOL:
          printf("Literal bool | ");
          break;
        case NATUREZA_LITERAL_STRING:
          printf("Literal string | ");
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
    printf("\n");
}
