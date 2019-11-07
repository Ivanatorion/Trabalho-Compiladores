#include "../include/iloc.h"
#include "../include/defines.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define DESLOC_PRINT 20

typedef struct var_end{
  int isGlobal;
  int desloc;
} VAR_END;

VAR_END getVarEnd(NODO_ARVORE *nodo, T_SIMBOLO* tabela){
  VAR_END result;

  if(strcmp(nodo->valor_lexico.valTokStr, "[]")){
    //Var simples
    
  }
  else{

  }

  return result;
}

char* newCCName(){
  static int nextCC = 1;

  char ccName[100];
  sprintf(ccName, "cc%d", nextCC);
  nextCC++;

  return strdup(ccName);
}

char* newRegName(){
  static int nextReg = 1;

  char regName[100];
  sprintf(regName, "r%d", nextReg);
  nextReg++;

  return strdup(regName);
}

char* newLabelName(){
  static int nextLabel = 1;

  char labelName[100];
  sprintf(labelName, "L%d", nextLabel);
  nextLabel++;

  return strdup(labelName);
}

void printInstructionList(ILOC_INST_LIST *instList){
  while(instList != NULL){
    printf("%s\n", instList->instruction->inst);
    instList = instList->prox;
  }
}

ILOC_INST_LIST* addInstructionToList(ILOC_INST_LIST *list, ILOC_INST *inst){
  ILOC_INST_LIST *result = list;

  if(result == NULL){
    result = malloc(sizeof(ILOC_INST_LIST));
    result->prox = NULL;
    result->instruction = inst;
  }
  else{
    while(list->prox != NULL)
      list = list->prox;

    list->prox = malloc(sizeof(ILOC_INST_LIST));
    list = list->prox;
    list->prox = NULL;
    list->instruction = inst;
  }

  return result;
}

ILOC_INST_LIST* concatInstructionLists(ILOC_INST_LIST *l1, ILOC_INST_LIST *l2){
  if(l1 == NULL)
    return l2;
  if(l2 == NULL)
    return l1;

  ILOC_INST_LIST *result = l1;

  while(l1->prox != NULL)
    l1 = l1->prox;

  l1->prox = l2;

  return result;
}

void genNodeCode(NODO_ARVORE* nodo, T_SIMBOLO* tabela){
  char buffer[128];

  if(nodo == NULL || nodo->instructionList != NULL)
    return;

  if(nodo->tipo == TL_BOOL){
    if(nodo->nFilhosMax == 3){
      if(!strcmp(nodo->valor_lexico.valTokStr, "=")){
          if(nodo->filhos[1]->tipo == TL_BOOL){
              char *ltrue = newLabelName();
              char *lfalse = newLabelName();
              char *lend = newLabelName();
              nodo->filhos[1]->bexpHatt.lt = ltrue;
              nodo->filhos[1]->bexpHatt.lf = lfalse;
              genNodeCode(nodo->filhos[1], tabela);
              nodo->instructionList = nodo->filhos[1]->instructionList;
              char *reg = newRegName();

              ILOC_INST_LIST *extras = NULL;
              ILOC_INST *instruction;

              instruction = malloc(sizeof(ILOC_INST));
              sprintf(buffer, "%s: loadI true => %s", ltrue, reg);
              instruction->inst = strdup(buffer);
              extras = addInstructionToList(extras, instruction);

              instruction = malloc(sizeof(ILOC_INST));
              sprintf(buffer, "jumpI -> %s", lend);
              instruction->inst = strdup(buffer);
              extras = addInstructionToList(extras, instruction);

              instruction = malloc(sizeof(ILOC_INST));
              sprintf(buffer, "%s: loadI false => %s", lfalse, reg);
              instruction->inst = strdup(buffer);
              extras = addInstructionToList(extras, instruction);

              instruction = malloc(sizeof(ILOC_INST));
              sprintf(buffer, "jumpI -> %s", lend);
              instruction->inst = strdup(buffer);
              extras = addInstructionToList(extras, instruction);

              VAR_END vend = getVarEnd(nodo->filhos[0], tabela);

              instruction = malloc(sizeof(ILOC_INST));
              sprintf(buffer, "%s: storeAI %s => %s, %d", lend, reg, (vend.isGlobal) ? "rbss" : "rfp", vend.desloc);
              instruction->inst = strdup(buffer);
              extras = addInstructionToList(extras, instruction);

              nodo->instructionList = concatInstructionLists(nodo->instructionList, extras);

              free(ltrue);
              free(lfalse);
              free(lend);
              free(reg);
          }
      }
      else if(!strcmp(nodo->valor_lexico.valTokStr, "&&")){
        nodo->filhos[0]->bexpHatt.lf = strdup(nodo->bexpHatt.lf);
        nodo->filhos[0]->bexpHatt.lt = newLabelName();
        genNodeCode(nodo->filhos[0], tabela);
        nodo->filhos[1]->bexpHatt.lf = strdup(nodo->bexpHatt.lf);
        nodo->filhos[1]->bexpHatt.lt = strdup(nodo->bexpHatt.lt);
        genNodeCode(nodo->filhos[1], tabela);
        sprintf(buffer, "%s: nop", nodo->filhos[0]->bexpHatt.lt);

        ILOC_INST *instruction = malloc(sizeof(ILOC_INST));
        instruction->inst = strdup(buffer);

        nodo->instructionList = addInstructionToList(nodo->filhos[0]->instructionList, instruction);
        nodo->instructionList = concatInstructionLists(nodo->instructionList, nodo->filhos[1]->instructionList);
      }
      else if(!strcmp(nodo->valor_lexico.valTokStr, "||")){
        nodo->filhos[0]->bexpHatt.lf = newLabelName();
        nodo->filhos[0]->bexpHatt.lt = strdup(nodo->bexpHatt.lt);
        genNodeCode(nodo->filhos[0], tabela);
        nodo->filhos[1]->bexpHatt.lf = strdup(nodo->bexpHatt.lf);
        nodo->filhos[1]->bexpHatt.lt = strdup(nodo->bexpHatt.lt);
        genNodeCode(nodo->filhos[1], tabela);
        sprintf(buffer, "%s: nop", nodo->filhos[0]->bexpHatt.lf);

        ILOC_INST *instruction = malloc(sizeof(ILOC_INST));
        instruction->inst = strdup(buffer);

        nodo->instructionList = addInstructionToList(nodo->filhos[0]->instructionList, instruction);
        nodo->instructionList = concatInstructionLists(nodo->instructionList, nodo->filhos[1]->instructionList);
      }
      else if(!strcmp(nodo->valor_lexico.valTokStr, "<")){
        ILOC_INST *instruction;
        genNodeCode(nodo->filhos[0], tabela);
        genNodeCode(nodo->filhos[1], tabela);
        nodo->instructionList = concatInstructionLists(nodo->filhos[0]->instructionList, nodo->filhos[1]->instructionList);

        char *ccN = newCCName();

        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "comp %s, %s => %s", nodo->filhos[0]->IlocRegName, nodo->filhos[1]->IlocRegName, ccN);
        instruction->inst = strdup(buffer);
        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "cbr_LT %s -> %s, %s", ccN, nodo->bexpHatt.lt, nodo->bexpHatt.lf);
        instruction->inst = strdup(buffer);
        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        free(ccN);
      }

    }
    else if(nodo->nFilhosMax == 2){ //UnOp
      if(!strcmp(nodo->valor_lexico.valTokStr, "!")){
        nodo->filhos[0]->bexpHatt.lf = strdup(nodo->bexpHatt.lt);
        nodo->filhos[0]->bexpHatt.lt = strdup(nodo->bexpHatt.lf);
        genNodeCode(nodo->filhos[0], tabela);
        nodo->instructionList = nodo->filhos[0]->instructionList;
      }
    }
    else{
      if(nodo->valor_lexico.tipo_literal == TL_BOOL){ //Literal
        ILOC_INST *instruction = malloc(sizeof(ILOC_INST));
        if(nodo->valor_lexico.valTokBool == 0)
          sprintf(buffer, "jumpI %s", nodo->bexpHatt.lf);
        else
          sprintf(buffer, "jumpI %s", nodo->bexpHatt.lt);

        instruction->inst = strdup(buffer);
        nodo->instructionList = malloc(sizeof(ILOC_INST_LIST));
        nodo->instructionList->prox = NULL;
        nodo->instructionList->instruction = instruction;
      }
      else{ //Identificador

      }
    }
  }
  else{
    for(int i = 0; i < nodo->nFilhosMax; i++){
      genNodeCode(nodo->filhos[i], tabela);
      if(nodo->filhos[i] != NULL)
        nodo->instructionList = concatInstructionLists(nodo->instructionList, nodo->filhos[i]->instructionList);
    }
  }

}

void genSaidaIloc(NODO_ARVORE* arvore, T_SIMBOLO* tabela){
  genNodeCode(arvore, tabela);

  printInstructionList(arvore->instructionList);

  FILE *fp = fopen("saida.iloc", "w");
  //Write...
  fclose(fp);
}
