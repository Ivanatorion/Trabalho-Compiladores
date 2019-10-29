#include "../include/iloc.h"
#include "../include/defines.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define DESLOC_PRINT 20

char* newRegName(){
  static int nextReg = 1;

  char regName[100];
  sprintf(regName, "r%d", nextReg);
  nextReg++;

  return strdup(regName);
}

void printInstructionList(ILOC_INST_LIST *instList){
  if(instList == NULL)
    return;

  ILOC_INST *inst = instList->instruction;

  if(instList->label != NULL){
    printf("%s:", instList->label);
    for(int i = strlen(instList->label) + 1; i < DESLOC_PRINT; i++)
      printf(" ");
  }
  else{
    for(int i = 0; i < DESLOC_PRINT; i++)
      printf(" ");
  }
  printf("[ ");

  while(inst != NULL){
    ILOC_OP *op = inst->operation;
    printf("%s %s, %s, %s ; ", op->opCode, op->op1, op->op2, op->op3);
    inst = inst->prox;
  }
  printf("]\n");

  printInstructionList(instList->prox);
}

void genBinOpCode(NODO_ARVORE* nodo, char *opName){
  ILOC_INST_LIST *listInst = NULL;
  ILOC_INST *iInst = NULL;

  char litOnString[100];
  char *r1, *r2, *r3;

  if(nodo->filhos[0]->valor_lexico.tipo_literal == TL_INT){
    if(nodo->filhos[1]->valor_lexico.tipo_literal == TL_INT){
      listInst = malloc(sizeof(ILOC_INST_LIST));
      iInst = malloc(sizeof(ILOC_INST));
      listInst->instruction = iInst;

      ILOC_OP *opLI = malloc(sizeof(ILOC_OP));

      sprintf(litOnString, "%d", nodo->filhos[1]->valor_lexico.valTokInt);

      opLI->opCode = strdup("loadI");
      opLI->op1 = strdup(litOnString);
      opLI->op2 = NULL;
      opLI->op3 = newRegName();
      r2 = opLI->op3;

      iInst->operation = opLI;
      iInst->prox = NULL;
    }
    else{
      r2 = strdup(nodo->filhos[1]->IlocOpName);
    }

    ILOC_OP *opLI2 = malloc(sizeof(ILOC_OP));
    if(iInst != NULL){
      iInst->prox = malloc(sizeof(ILOC_INST));
      iInst = iInst->prox;
    }
    else{
      listInst = malloc(sizeof(ILOC_INST_LIST));
      iInst = malloc(sizeof(ILOC_INST));
      listInst->instruction = iInst;
    }

    sprintf(litOnString, "%sI", opName);
    opLI2->opCode = strdup(litOnString);
    opLI2->op1 = r2;

    sprintf(litOnString, "%d", nodo->filhos[0]->valor_lexico.valTokInt);

    opLI2->op2 = strdup(litOnString);
    opLI2->op3 = newRegName();

    iInst->operation = opLI2;
    iInst->prox = NULL;

    listInst->prox = NULL;
  }
  else{
    if(nodo->filhos[1]->valor_lexico.tipo_literal == TL_INT){
      r1 = strdup(nodo->filhos[0]->IlocOpName);
      ILOC_OP *opLI = malloc(sizeof(ILOC_OP));
      listInst = malloc(sizeof(ILOC_INST_LIST));
      iInst = malloc(sizeof(ILOC_INST));
      listInst->instruction = iInst;

      sprintf(litOnString, "%sI", opName);
      opLI->opCode = strdup(litOnString);
      opLI->op1 = r1;

      sprintf(litOnString, "%d", nodo->filhos[1]->valor_lexico.valTokInt);

      opLI->op2 = strdup(litOnString);
      opLI->op3 = newRegName();

      iInst->operation = opLI;
      iInst->prox = NULL;

      listInst->prox = NULL;
    }
    else{
      ILOC_OP *opLI = malloc(sizeof(ILOC_OP));
      listInst = malloc(sizeof(ILOC_INST_LIST));
      iInst = malloc(sizeof(ILOC_INST));
      listInst->instruction = iInst;

      sprintf(litOnString, "%s", opName);
      opLI->opCode = strdup(litOnString);
      opLI->op1 = strdup(nodo->filhos[0]->IlocOpName);
      opLI->op2 = strdup(nodo->filhos[1]->IlocOpName);
      opLI->op3 = newRegName();
    }
  }
  listInst->prox = NULL;
  listInst->label = NULL;
  printInstructionList(listInst);

  printf("\n\n\n\n");
}

void genIdOpCode(NODO_ARVORE* nodo, T_SIMBOLO *tabela){
  ILOC_INST_LIST *list = malloc(sizeof(ILOC_INST_LIST));
  ILOC_INST *inst = malloc(sizeof(ILOC_INST));
  ILOC_OP *op = malloc(sizeof(ILOC_OP));

  char litOnString[100];

  list->prox = NULL;
  list->instruction = inst;

  //Certamente esta na tabela
  S_INFO sInfo;
  consulta_tabela(tabela, nodo->valor_lexico.valTokStr, &sInfo);

  op->opCode = strdup("loadAI");
  op->op1 = (sInfo.isVarGlob) ? strdup("rbss") : strdup("rfp");
  sprintf(litOnString, "%d", sInfo.varDesloc);
  op->op2 = strdup(litOnString);
  op->op3 = newRegName();

  inst->operation = op;
  inst->prox = NULL;

  nodo->instructionList = list;
}

void genLitOpCode(NODO_ARVORE* nodo){
  ILOC_INST_LIST *list = malloc(sizeof(ILOC_INST_LIST));
  ILOC_INST *inst = malloc(sizeof(ILOC_INST));
  ILOC_OP *op = malloc(sizeof(ILOC_OP));

  nodo->IlocOpName = newRegName();

  list->instruction = inst;

  inst->operation = op;

  
}

void genNodeCode(NODO_ARVORE* nodo, T_SIMBOLO* tabela){
  if(nodo == NULL || nodo->IlocOpName != NULL)
    return;

  for(int i = 0; i < nodo->nFilhosMax; i++){
    printf("Gen\n");
    genNodeCode(nodo->filhos[i], tabela);
  }

  if(nodo->valor_lexico.tipo_literal == TL_NONE){
    if(nodo->valor_lexico.tipo_token == TT_ID){
      genIdOpCode(nodo, tabela);
    }

    if(!strcmp(nodo->valor_lexico.valTokStr, "+") && nodo->nFilhosMax == 3){
      genBinOpCode(nodo, "add");
    }
  }
  else{
    genLitOpCode(nodo);
  }

}

void genSaidaIloc(NODO_ARVORE* arvore, T_SIMBOLO* tabela){
  genNodeCode(arvore, tabela);

  FILE *fp = fopen("saida.iloc", "w");
  //Write...
  fclose(fp);
}
