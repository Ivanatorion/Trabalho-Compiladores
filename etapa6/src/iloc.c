#include "../include/iloc.h"
#include "../include/defines.h"
#include "../include/labeltable.h"
#include "../include/optimizer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern LABEL_TABLE *label_table;

typedef struct var_end{
  int isGlobal;
  int isAbsolute;

  int absoluteDesloc;
  char *regDesloc;
  ILOC_INST_LIST *regDeslocCalc;
} VAR_END;

typedef struct rlist{
  struct rlist *prox;
  struct rlist *ant;

  char *reg;
} REG_LIST;

char* newRegName(){
  static int nextReg = 1;

  char regName[256];
  sprintf(regName, "r%d", nextReg);
  nextReg++;

  return strdup(regName);
}

char* newLabelName(){
  static int nextLabel = 1;

  char labelName[256];
  sprintf(labelName, "L%d", nextLabel);
  nextLabel++;

  return strdup(labelName);
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
  if(l1 == l2)
    return l1;

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

void printInstructionList(FILE *out, ILOC_INST_LIST *instList){
  while(instList != NULL){
    fprintf(out, "%s\n", instList->instruction->inst);
    instList = instList->prox;
  }
}

REG_LIST* addReg(REG_LIST* rList, char* reg){
  const char blackList[4][10] = {"rfp", "rsp", "rpc", "rbss"};

  if(reg == NULL)
    return rList;

  printf("Reg: %s\n", reg);

  int blackListed = 0;
  for(int i = 0; i < 4; i++)
    if(!strcmp(reg, blackList[i]))
      blackListed = 1;

  if(blackListed)
    return rList;

  if(rList == NULL){
    rList = malloc(sizeof(REG_LIST));
    rList->prox = NULL;
    rList->ant = NULL;
    rList->reg = strdup(reg);
    return rList;
  }

  REG_LIST *auxL = rList;
  while(auxL->prox != NULL)
    auxL = auxL->prox;

  auxL->prox = malloc(sizeof(REG_LIST));
  auxL->prox->prox = NULL;
  auxL->prox->ant = auxL;
  auxL->prox->reg = strdup(reg);
  return rList;
}

void freeRegList(REG_LIST *rList){
  if(rList == NULL)
    return;

  REG_LIST *aux;

  do {
    aux = rList->prox;
    free(rList->reg);
    free(rList);
    rList = aux;
  } while (rList != NULL);
}

void genPushCode(ILOC_INST_LIST *iList, REG_LIST *rList){
  if(rList == NULL){
    free(iList->instruction->inst);
    free(iList->instruction);

    iList->instruction = iList->prox->instruction;

    ILOC_INST_LIST *auxIL = iList->prox;
    iList->prox = iList->prox->prox;
    free(auxIL);

    return;
  }

  ILOC_INST_LIST *newL = NULL;
  ILOC_INST *newInst = NULL;

  char buffer[128];
  int desloc = 0;
  while(rList != NULL){
    newInst = malloc(sizeof(ILOC_INST));
    sprintf(buffer, "storeAI %s => rsp, %d", rList->reg, desloc);
    newInst->inst = strdup(buffer);
    newL = addInstructionToList(newL, newInst);
    rList = rList->prox;
    desloc = desloc + 4;
  }
  newInst = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "addI rsp, %d => rsp", desloc);
  newInst->inst = strdup(buffer);
  newL = addInstructionToList(newL, newInst);

  sprintf(iList->instruction->inst, "nop");
  newL = concatInstructionLists(newL, iList->prox);
  iList->prox = newL;
}

void genPushCode(ILOC_INST_LIST *iList, REG_LIST *rList){
  //Lembrar de nao popar o ultimo adicionado

}

void fixPushPopRegisters(ILOC_INST_LIST *iList){
  const char regLoadInstructions[5][10] = {"load", "add", "mult", "sub", "div"};

  REG_LIST *rList = NULL;

  ILOC_INST_LIST *cInst = iList;

  while(cInst != NULL){
    char *instString = cInst->instruction->inst;
    int addRegToList = 0;

    for(int i = 0; i < 5; i++){
      if(strstr(instString, regLoadInstructions[i])){
        addRegToList = 1;
      }
    }

    if(addRegToList){
      char *regToAdd = strstr(instString, "=> ") + 3;
      rList = addReg(rList, regToAdd);
    }

    if(!strcmp(instString, PUSH_MARK)){
      genPushCode(cInst, rList);
    }

    if(!strcmp(instString, POP_MARK)){
      genPopCode(cInst, rList);
    }

    cInst = cInst->prox;
  }

  /*
  REG_LIST *auxL = rList;

  printf("Reg List:\n");
  while(auxL != NULL){
    printf("%s\n", auxL->reg);
    auxL = auxL->prox;
  }
  */

  freeRegList(rList);
}

VAR_END getVarEnd(NODO_ARVORE *nodo, T_SIMBOLO* tabela){
  VAR_END result;
  S_INFO sInfo;

  ILOC_INST *instruction;

  char buffer[128];

  while(tabela->ant != NULL)
    tabela = tabela->ant;

  if(strcmp(nodo->valor_lexico.valTokStr, "[]")){
    //Var simples
    result.isAbsolute = 1;
    consulta_tabela(tabela, nodo->valor_lexico.valTokStr, &sInfo);
    result.absoluteDesloc = sInfo.varDesloc;
  }
  else{
    result.isAbsolute = 0;
    consulta_tabela(tabela, nodo->filhos[0]->valor_lexico.valTokStr, &sInfo);

    nodo = nodo->filhos[1];

    result.regDesloc = newRegName();
    char *auxR;

    instruction = malloc(sizeof(ILOC_INST));
    sprintf(buffer, "loadI 0 => %s", result.regDesloc);
    instruction->inst = strdup(buffer);
    result.regDeslocCalc = malloc(sizeof(ILOC_INST_LIST));
    result.regDeslocCalc->prox = NULL;
    result.regDeslocCalc->instruction = instruction;

    genNodeCode(nodo, tabela);
    result.regDeslocCalc = concatInstructionLists(result.regDeslocCalc, nodo->instructionList);

    int acDim;

    while(nodo != NULL){
      acDim = sInfo.dimList->dim;

      auxR = result.regDesloc;
      result.regDesloc = newRegName();
      instruction = malloc(sizeof(ILOC_INST));
      sprintf(buffer, "multI %s, %d => %s", auxR, acDim, result.regDesloc);
      instruction->inst = strdup(buffer);
      result.regDeslocCalc = addInstructionToList(result.regDeslocCalc, instruction);
      free(auxR);

      auxR = result.regDesloc;
      result.regDesloc = newRegName();
      instruction = malloc(sizeof(ILOC_INST));
      sprintf(buffer, "add %s, %s => %s", auxR, nodo->IlocRegName, result.regDesloc);
      instruction->inst = strdup(buffer);
      result.regDeslocCalc = addInstructionToList(result.regDeslocCalc, instruction);
      free(auxR);

      sInfo.dimList = sInfo.dimList->prox;
      nodo = nodo->filhos[nodo->nFilhosMax-1];
    }

    auxR = result.regDesloc;
    result.regDesloc = newRegName();
    instruction = malloc(sizeof(ILOC_INST));
    sprintf(buffer, "multI %s, 4 => %s", auxR, result.regDesloc);
    instruction->inst = strdup(buffer);
    result.regDeslocCalc = addInstructionToList(result.regDeslocCalc, instruction);
    free(auxR);

    auxR = result.regDesloc;
    result.regDesloc = newRegName();
    instruction = malloc(sizeof(ILOC_INST));
    sprintf(buffer, "addI %s, %d => %s", auxR, sInfo.varDesloc, result.regDesloc);
    instruction->inst = strdup(buffer);
    result.regDeslocCalc = addInstructionToList(result.regDeslocCalc, instruction);
    free(auxR);
  }

  result.isGlobal = sInfo.isVarGlob;
  return result;
}

void genNodeCodeIntBinop(NODO_ARVORE* nodo, T_SIMBOLO* tabela){
  char buffer[128];
  char instName[20];
  ILOC_INST *instruction;

  genNodeCode(nodo->filhos[0], tabela);
  genNodeCode(nodo->filhos[1], tabela);
  nodo->instructionList = concatInstructionLists(nodo->filhos[0]->instructionList, nodo->filhos[1]->instructionList);
  nodo->IlocRegName = newRegName();

  if(!strcmp(nodo->valor_lexico.valTokStr, "+"))
    sprintf(instName, "add");
  else if(!strcmp(nodo->valor_lexico.valTokStr, "-"))
    sprintf(instName, "sub");
  else if(!strcmp(nodo->valor_lexico.valTokStr, "*"))
    sprintf(instName, "mult");
  else if(!strcmp(nodo->valor_lexico.valTokStr, "/"))
    sprintf(instName, "div");
  else
    sprintf(instName, "ERR");

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "%s %s, %s => %s", instName, nodo->filhos[0]->IlocRegName, nodo->filhos[1]->IlocRegName, nodo->IlocRegName);
  instruction->inst = strdup(buffer);
  nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);
}

void genNodeCodeBoolCompare(NODO_ARVORE* nodo, T_SIMBOLO* tabela){
  ILOC_INST *instruction;
  char buffer[128];

  genNodeCode(nodo->filhos[0], tabela);
  genNodeCode(nodo->filhos[1], tabela);
  nodo->instructionList = concatInstructionLists(nodo->filhos[0]->instructionList, nodo->filhos[1]->instructionList);

  char compStr[20];
  if(!strcmp(nodo->valor_lexico.valTokStr, "<"))
    sprintf(compStr, "LT");
  else if(!strcmp(nodo->valor_lexico.valTokStr, ">"))
    sprintf(compStr, "GT");
  else if(!strcmp(nodo->valor_lexico.valTokStr, "!="))
    sprintf(compStr, "NE");
  else if(!strcmp(nodo->valor_lexico.valTokStr, "=="))
    sprintf(compStr, "EQ");
  else if(!strcmp(nodo->valor_lexico.valTokStr, "<="))
    sprintf(compStr, "LE");
  else if(!strcmp(nodo->valor_lexico.valTokStr, ">="))
    sprintf(compStr, "GE");

  char *ccN = newRegName();

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "cmp_%s %s, %s -> %s", compStr, nodo->filhos[0]->IlocRegName, nodo->filhos[1]->IlocRegName, ccN);
  instruction->inst = strdup(buffer);
  nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "cbr %s -> %s, %s", ccN, nodo->bexpHatt.lt, nodo->bexpHatt.lf);
  instruction->inst = strdup(buffer);
  nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

  free(ccN);
}

void genNodeCodeAtrib(NODO_ARVORE* nodo, T_SIMBOLO* tabela){
    char buffer[128];

    ILOC_INST *instruction;

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

        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "%s: loadI 1 => %s", ltrue, reg);
        instruction->inst = strdup(buffer);
        extras = addInstructionToList(extras, instruction);

        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "jumpI -> %s", lend);
        instruction->inst = strdup(buffer);
        extras = addInstructionToList(extras, instruction);

        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "%s: loadI 0 => %s", lfalse, reg);
        instruction->inst = strdup(buffer);
        extras = addInstructionToList(extras, instruction);

        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "jumpI -> %s", lend);
        instruction->inst = strdup(buffer);
        extras = addInstructionToList(extras, instruction);

        VAR_END vend = getVarEnd(nodo->filhos[0], tabela);

        if(vend.isAbsolute){
          instruction = malloc(sizeof(ILOC_INST));
          sprintf(buffer, "%s: storeAI %s => %s, %d", lend, reg, (vend.isGlobal) ? "rbss" : "rfp", vend.absoluteDesloc);
          instruction->inst = strdup(buffer);
          extras = addInstructionToList(extras, instruction);
        }
        else{
          instruction = malloc(sizeof(ILOC_INST));
          sprintf(buffer, "%s: nop", lend);
          instruction->inst = strdup(buffer);
          extras = addInstructionToList(extras, instruction);

          extras = concatInstructionLists(extras, vend.regDeslocCalc);

          char *auxR = newRegName();

          instruction = malloc(sizeof(ILOC_INST));
          sprintf(buffer, "add %s, %s => %s", (vend.isGlobal) ? "rbss" : "rfp", vend.regDesloc, auxR);
          instruction->inst = strdup(buffer);
          extras = addInstructionToList(extras, instruction);

          instruction = malloc(sizeof(ILOC_INST));
          sprintf(buffer, "store %s => %s", reg, auxR);
          instruction->inst = strdup(buffer);
          extras = addInstructionToList(extras, instruction);

          free(auxR);
          free(vend.regDesloc);
        }

        nodo->instructionList = concatInstructionLists(nodo->instructionList, extras);

        free(ltrue);
        free(lfalse);
        free(lend);
        free(reg);
    }
    else if(nodo->filhos[1]->tipo == TL_INT){
      VAR_END vend = getVarEnd(nodo->filhos[0], tabela);
      genNodeCode(nodo->filhos[1], tabela);
      nodo->instructionList = nodo->filhos[1]->instructionList;

      if(vend.isAbsolute){
        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "storeAI %s => %s, %d", nodo->filhos[1]->IlocRegName, (vend.isGlobal) ? "rbss" : "rfp", vend.absoluteDesloc);
        instruction->inst = strdup(buffer);
        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);
      }
      else{
        nodo->instructionList = concatInstructionLists(nodo->instructionList, vend.regDeslocCalc);

        char *auxR = newRegName();

        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "add %s, %s => %s", (vend.isGlobal) ? "rbss" : "rfp", vend.regDesloc, auxR);
        instruction->inst = strdup(buffer);
        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "store %s => %s", nodo->filhos[1]->IlocRegName, auxR);
        instruction->inst = strdup(buffer);
        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        free(auxR);
        free(vend.regDesloc);
      }
    }
}

void genNodeCodeWhile(NODO_ARVORE* nodo, T_SIMBOLO* tabela){
  char buffer[128];

  char *beginWhile = newLabelName();
  char *beginCode = newLabelName();
  char *endWhile = newLabelName();

  ILOC_INST *instruction;

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "%s: nop", beginWhile);
  instruction->inst = strdup(buffer);
  nodo->instructionList = malloc(sizeof(ILOC_INST_LIST));
  nodo->instructionList->instruction = instruction;
  nodo->instructionList->prox = NULL;

  nodo->filhos[0]->bexpHatt.lt = beginCode;
  nodo->filhos[0]->bexpHatt.lf = endWhile;
  genNodeCode(nodo->filhos[0], tabela);

  nodo->instructionList = concatInstructionLists(nodo->instructionList, nodo->filhos[0]->instructionList);

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "%s: nop", beginCode);
  instruction->inst = strdup(buffer);
  nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

  genNodeCode(nodo->filhos[1], tabela);
  nodo->instructionList = concatInstructionLists(nodo->instructionList, nodo->filhos[1]->instructionList);

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "jumpI -> %s", beginWhile);
  instruction->inst = strdup(buffer);
  nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "%s: nop", endWhile);
  instruction->inst = strdup(buffer);
  nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

  free(beginWhile);
  free(beginCode);
  free(endWhile);
}

void genNodeCodeReturn(NODO_ARVORE* nodo, T_SIMBOLO* tabela){
  char buffer[128];
  ILOC_INST *instruction;

  genNodeCode(nodo->filhos[0], tabela);

  nodo->instructionList = nodo->filhos[0]->instructionList;

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "storeAI %s => rfp, %d", nodo->filhos[0]->IlocRegName, RET_VAL_FRAME_DESLOC);
  instruction->inst = strdup(buffer);
  nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "i2i rfp => rsp");
  instruction->inst = strdup(buffer);
  nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "loadAI rfp, 0 => rfp");
  instruction->inst = strdup(buffer);
  nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

  char *auxR = newRegName();

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "loadAI rsp, 4 => %s", auxR);
  instruction->inst = strdup(buffer);
  nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "addI %s, %d => %s", auxR, PC_RETURN_ADD, auxR);
  instruction->inst = strdup(buffer);
  nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "jump -> %s", auxR);
  instruction->inst = strdup(buffer);
  nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

  free(auxR);
}

void genNodeCode(NODO_ARVORE* nodo, T_SIMBOLO* tabela){
  char buffer[128];
  int isFunction = 0; //Indica se o "nodo" eh funcao

  if(nodo == NULL || nodo->instructionList != NULL)
    return;

  if(nodo->tipo == TL_BOOL){
    if(nodo->nFilhosMax == 3){
      if(!strcmp(nodo->valor_lexico.valTokStr, "=")){
        genNodeCodeAtrib(nodo, tabela);
      }
      else if(!strcmp(nodo->valor_lexico.valTokStr, "&&")){
        nodo->filhos[0]->bexpHatt.lf = nodo->bexpHatt.lf;
        nodo->filhos[0]->bexpHatt.lt = newLabelName();
        genNodeCode(nodo->filhos[0], tabela);
        nodo->filhos[1]->bexpHatt.lf = nodo->bexpHatt.lf;
        nodo->filhos[1]->bexpHatt.lt = nodo->bexpHatt.lt;
        genNodeCode(nodo->filhos[1], tabela);
        sprintf(buffer, "%s: nop", nodo->filhos[0]->bexpHatt.lt);

        ILOC_INST *instruction = malloc(sizeof(ILOC_INST));
        instruction->inst = strdup(buffer);

        nodo->instructionList = addInstructionToList(nodo->filhos[0]->instructionList, instruction);
        nodo->instructionList = concatInstructionLists(nodo->instructionList, nodo->filhos[1]->instructionList);

        free(nodo->filhos[0]->bexpHatt.lt);
      }
      else if(!strcmp(nodo->valor_lexico.valTokStr, "||")){
        nodo->filhos[0]->bexpHatt.lf = newLabelName();
        nodo->filhos[0]->bexpHatt.lt = nodo->bexpHatt.lt;
        genNodeCode(nodo->filhos[0], tabela);
        nodo->filhos[1]->bexpHatt.lf = nodo->bexpHatt.lf;
        nodo->filhos[1]->bexpHatt.lt = nodo->bexpHatt.lt;
        genNodeCode(nodo->filhos[1], tabela);
        sprintf(buffer, "%s: nop", nodo->filhos[0]->bexpHatt.lf);

        ILOC_INST *instruction = malloc(sizeof(ILOC_INST));
        instruction->inst = strdup(buffer);

        nodo->instructionList = addInstructionToList(nodo->filhos[0]->instructionList, instruction);
        nodo->instructionList = concatInstructionLists(nodo->instructionList, nodo->filhos[1]->instructionList);
        free(nodo->filhos[0]->bexpHatt.lf);
      }
      else if(!strcmp(nodo->valor_lexico.valTokStr, "[]")){
        VAR_END vend = getVarEnd(nodo, tabela);
        nodo->IlocRegName = newRegName();
        ILOC_INST *instruction;

        nodo->instructionList = vend.regDeslocCalc;

        char *auxR = newRegName();

        sprintf(buffer, "add %s, %s => %s", (vend.isGlobal) ? "rbss" : "rfp", vend.regDesloc, auxR);
        instruction = malloc(sizeof(ILOC_INST));
        instruction->inst = strdup(buffer);
        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        sprintf(buffer, "load %s => %s", auxR, nodo->IlocRegName);
        instruction = malloc(sizeof(ILOC_INST));
        instruction->inst = strdup(buffer);
        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        free(auxR);

        sprintf(buffer, "cbr %s -> %s, %s", nodo->IlocRegName, nodo->bexpHatt.lt, nodo->bexpHatt.lf);
        instruction = malloc(sizeof(ILOC_INST));
        instruction->inst = strdup(buffer);
        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        free(vend.regDesloc);
      }
      else{
        genNodeCodeBoolCompare(nodo, tabela);
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
          sprintf(buffer, "jumpI -> %s", nodo->bexpHatt.lf);
        else
          sprintf(buffer, "jumpI -> %s", nodo->bexpHatt.lt);

        instruction->inst = strdup(buffer);
        nodo->instructionList = malloc(sizeof(ILOC_INST_LIST));
        nodo->instructionList->prox = NULL;
        nodo->instructionList->instruction = instruction;
      }
      else{ //Identificador
        VAR_END vend = getVarEnd(nodo, tabela);
        nodo->IlocRegName = newRegName();
        ILOC_INST *instruction;

        sprintf(buffer, "loadAI %s, %d => %s", (vend.isGlobal) ? "rbss" : "rfp", vend.absoluteDesloc, nodo->IlocRegName);
        instruction = malloc(sizeof(ILOC_INST));
        instruction->inst = strdup(buffer);
        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        sprintf(buffer, "cbr %s -> %s, %s", nodo->IlocRegName, nodo->bexpHatt.lt, nodo->bexpHatt.lf);
        instruction = malloc(sizeof(ILOC_INST));
        instruction->inst = strdup(buffer);
        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);
      }
    }
  }
  else if(nodo->tipo == TL_INT){
    ILOC_INST *instruction;

    if(nodo->nFilhosMax == 3){
      if(!strcmp(nodo->valor_lexico.valTokStr, "=")){
        genNodeCodeAtrib(nodo, tabela);
      }
      else if(!strcmp(nodo->valor_lexico.valTokStr, "[]")){
        VAR_END vend = getVarEnd(nodo, tabela);

        nodo->instructionList = vend.regDeslocCalc;

        char *auxR = newRegName();
        nodo->IlocRegName = newRegName();

        sprintf(buffer, "add %s, %s => %s", (vend.isGlobal) ? "rbss" : "rfp", vend.regDesloc, auxR);
        instruction = malloc(sizeof(ILOC_INST));
        instruction->inst = strdup(buffer);
        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        sprintf(buffer, "load %s => %s", auxR, nodo->IlocRegName);
        instruction = malloc(sizeof(ILOC_INST));
        instruction->inst = strdup(buffer);
        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        free(auxR);
        free(vend.regDesloc);
      }
      else{
        genNodeCodeIntBinop(nodo, tabela);
      }
    }
    else if(nodo->nFilhosMax == 1){
      if(nodo->valor_lexico.tipo_literal == TL_NONE){
        VAR_END vend = getVarEnd(nodo, tabela);
        nodo->IlocRegName = newRegName();
        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "loadAI %s, %d => %s", (vend.isGlobal) ? "rbss" : "rfp", vend.absoluteDesloc, nodo->IlocRegName);
        instruction->inst = strdup(buffer);
        nodo->instructionList = malloc(sizeof(ILOC_INST_LIST));
        nodo->instructionList->instruction = instruction;
        nodo->instructionList->prox = NULL;
      }
      else if(nodo->valor_lexico.tipo_literal == TL_INT){
        nodo->IlocRegName = newRegName();
        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "loadI %d => %s", nodo->valor_lexico.valTokInt, nodo->IlocRegName);
        instruction->inst = strdup(buffer);
        nodo->instructionList = malloc(sizeof(ILOC_INST_LIST));
        nodo->instructionList->instruction = instruction;
        nodo->instructionList->prox = NULL;
      }
    }
    else{ //Chamada de funcao
      S_INFO sInfo;
      consulta_tabela(tabela, nodo->valor_lexico.valTokStr, &sInfo);

      //Argumentos
      NODO_ARVORE* params = nodo->filhos[0];
      if(params != NULL){
        genNodeCode(params, tabela);
        nodo->instructionList = concatInstructionLists(nodo->instructionList, params->instructionList);
      }

      //Push registers mark
      instruction = malloc(sizeof(ILOC_INST));
      instruction->inst = strdup(PUSH_MARK);
      nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

      int spDesloc = STACK_FRAME_TAM_FIX;
      while(params != NULL){
        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "storeAI %s => rsp, %d", params->IlocRegName, spDesloc);
        instruction->inst = strdup(buffer);
        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        spDesloc = spDesloc + 4;
        params = params->filhos[params->nFilhosMax - 1];
      }

      //FP
      instruction = malloc(sizeof(ILOC_INST));
      sprintf(buffer, "storeAI rfp => rsp, 0");
      instruction->inst = strdup(buffer);
      nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

      //PC
      instruction = malloc(sizeof(ILOC_INST));
      sprintf(buffer, "storeAI rpc => rsp, 4");
      instruction->inst = strdup(buffer);
      nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

      //Move frame
      instruction = malloc(sizeof(ILOC_INST));
      sprintf(buffer, "i2i rsp => rfp");
      instruction->inst = strdup(buffer);
      nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

      //Jump
      if(label_table == NULL)
        label_table = make_label_table();

      LABEL_TABLE_ENTRY* fEntry = consulta_label_table(label_table, nodo->valor_lexico.valTokStr);

      char* fLabel;
      if(fEntry == NULL){
        fLabel = newLabelName();
        insere_label_table(label_table, nodo->valor_lexico.valTokStr, fLabel, 0);
        sprintf(buffer, "jumpI -> %s", fLabel);
        free(fLabel);
      }
      else{
        fLabel = fEntry->value;
        sprintf(buffer, "jumpI -> %s", fLabel);
      }

      instruction = malloc(sizeof(ILOC_INST));
      instruction->inst = strdup(buffer);
      nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

      //Pega retorno
      nodo->IlocRegName = newRegName();

      instruction = malloc(sizeof(ILOC_INST));
      sprintf(buffer, "loadAI rsp, %d => %s", RET_VAL_FRAME_DESLOC, nodo->IlocRegName);
      instruction->inst = strdup(buffer);
      nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

      //Pop registers mark
      instruction = malloc(sizeof(ILOC_INST));
      instruction->inst = strdup(POP_MARK);
      nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);
    }
  }
  else if(nodo->tipo == TL_NONE){
    if(!strcmp(nodo->valor_lexico.valTokStr, "if")){
      if(nodo->nFilhosMax == 3){
        char *ltrue = newLabelName();
        char *lfalse = newLabelName();

        nodo->filhos[0]->bexpHatt.lt = ltrue;
        nodo->filhos[0]->bexpHatt.lf = lfalse;
        genNodeCode(nodo->filhos[0], tabela);

        genNodeCode(nodo->filhos[1], tabela);

        ILOC_INST *instruction;

        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "%s: nop", ltrue);
        instruction->inst = strdup(buffer);

        nodo->instructionList = nodo->filhos[0]->instructionList;
        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        nodo->instructionList = concatInstructionLists(nodo->instructionList, nodo->filhos[1]->instructionList);
        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "%s: nop", lfalse);
        instruction->inst = strdup(buffer);

        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        free(ltrue);
        free(lfalse);
      }
      else{
        char *ltrue = newLabelName();
        char *lfalse = newLabelName();
        char *lend = newLabelName();

        genNodeCode(nodo->filhos[1], tabela);
        if(nodo->nFilhosMax == 4)
          genNodeCode(nodo->filhos[2], tabela);

        nodo->filhos[0]->bexpHatt.lt = ltrue;
        nodo->filhos[0]->bexpHatt.lf = lfalse;
        genNodeCode(nodo->filhos[0], tabela);

        ILOC_INST *instruction;

        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "%s: nop", ltrue);
        instruction->inst = strdup(buffer);

        nodo->instructionList = addInstructionToList(nodo->filhos[0]->instructionList, instruction);
        nodo->instructionList = concatInstructionLists(nodo->instructionList, nodo->filhos[1]->instructionList);

        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "jumpI -> %s", lend);
        instruction->inst = strdup(buffer);

        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "%s: nop", lfalse);
        instruction->inst = strdup(buffer);

        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);
        nodo->instructionList = concatInstructionLists(nodo->instructionList, nodo->filhos[2]->instructionList);

        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "jumpI -> %s", lend);
        instruction->inst = strdup(buffer);

        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        instruction = malloc(sizeof(ILOC_INST));
        sprintf(buffer, "%s: nop", lend);
        instruction->inst = strdup(buffer);

        nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

        free(ltrue);
        free(lfalse);
        free(lend);
      }
    }
    else if(!strcmp(nodo->valor_lexico.valTokStr, "while")){
      genNodeCodeWhile(nodo, tabela);
    }
    else if(!strcmp(nodo->valor_lexico.valTokStr, "return")){
      genNodeCodeReturn(nodo, tabela);
    }
    else{
      for(int i = 0; i < nodo->nFilhosMax; i++){
        genNodeCode(nodo->filhos[i], tabela);
        if(nodo->filhos[i] != NULL)
          nodo->instructionList = concatInstructionLists(nodo->instructionList, nodo->filhos[i]->instructionList);
      }
    }
  }
  else{
    S_INFO sinfo;
    consulta_tabela(tabela, nodo->valor_lexico.valTokStr, &sinfo);

    if(sinfo.tipoIdentificador == TID_FUNC)
      isFunction = 1;

    if(isFunction){
      if(label_table == NULL)
        label_table = make_label_table();

      LABEL_TABLE_ENTRY* fEntry = consulta_label_table(label_table, nodo->valor_lexico.valTokStr);

      char* fLabel;

      if(fEntry == NULL){
        fLabel = newLabelName();
        insere_label_table(label_table, nodo->valor_lexico.valTokStr, fLabel, 0);
        sprintf(buffer, "%s: nop", fLabel);
        free(fLabel);
      }
      else{
        fLabel = fEntry->value;
        sprintf(buffer, "%s: nop", fLabel);
      }

      ILOC_INST* funcInit = malloc(sizeof(ILOC_INST));
      ILOC_INST* instruction;

      funcInit->inst = strdup(buffer);
      nodo->instructionList = malloc(sizeof(ILOC_INST_LIST));
      nodo->instructionList->instruction = funcInit;
      nodo->instructionList->prox = NULL;

      //Prologo
      instruction = malloc(sizeof(ILOC_INST));
      sprintf(buffer, "addI rsp, %d => rsp", consulta_label_table(label_table, nodo->valor_lexico.valTokStr)->accDesloc);
      instruction->inst = strdup(buffer);
      nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

      //Codigo
      genNodeCode(nodo->filhos[0], tabela);
      nodo->instructionList = concatInstructionLists(nodo->instructionList, nodo->filhos[0]->instructionList);

      //Epilogo (return 0)
      char *auxR = newRegName();

      instruction = malloc(sizeof(ILOC_INST));
      sprintf(buffer, "loadI 0 => %s", auxR);
      instruction->inst = strdup(buffer);
      nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

      instruction = malloc(sizeof(ILOC_INST));
      sprintf(buffer, "storeAI %s => rfp, %d", auxR, RET_VAL_FRAME_DESLOC);
      instruction->inst = strdup(buffer);
      nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

      instruction = malloc(sizeof(ILOC_INST));
      sprintf(buffer, "i2i rfp => rsp");
      instruction->inst = strdup(buffer);
      nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

      instruction = malloc(sizeof(ILOC_INST));
      sprintf(buffer, "loadAI rfp, 0 => rfp");
      instruction->inst = strdup(buffer);
      nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

      free(auxR);
      auxR = newRegName();

      instruction = malloc(sizeof(ILOC_INST));
      sprintf(buffer, "loadAI rsp, 4 => %s", auxR);
      instruction->inst = strdup(buffer);
      nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

      instruction = malloc(sizeof(ILOC_INST));
      sprintf(buffer, "addI %s, %d => %s", auxR, PC_RETURN_ADD, auxR);
      instruction->inst = strdup(buffer);
      nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

      instruction = malloc(sizeof(ILOC_INST));
      sprintf(buffer, "jump -> %s", auxR);
      instruction->inst = strdup(buffer);
      nodo->instructionList = addInstructionToList(nodo->instructionList, instruction);

      free(auxR);

      //Proximas Funcoes
      if(nodo->filhos[1] != NULL){
        genNodeCode(nodo->filhos[1], tabela);
        nodo->instructionList = concatInstructionLists(nodo->instructionList, nodo->filhos[1]->instructionList);
      }
    }
  }

  if(!isFunction && nodo->filhos[nodo->nFilhosMax - 1] != NULL){
    genNodeCode(nodo->filhos[nodo->nFilhosMax - 1], tabela);
    nodo->instructionList = concatInstructionLists(nodo->instructionList, nodo->filhos[nodo->nFilhosMax - 1]->instructionList);
  }

}

ILOC_INST_LIST* genFirstInstructions(T_SIMBOLO* tabela){
  char buffer[128];

  LABEL_TABLE_ENTRY* main_entry = consulta_label_table(label_table, "main");

  if(main_entry == NULL)
    return NULL;

  char* main_label = main_entry->value;

  S_INFO sInfo;
  if(tabela != NULL)
    consulta_tabela(tabela, "main", &sInfo);

  ILOC_INST_LIST* firstInstructions = malloc(sizeof(ILOC_INST_LIST));
  ILOC_INST* instruction;

  while(tabela->ant != NULL)
    tabela = tabela->ant;

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "loadI 0 => rbss");
  instruction->inst = strdup(buffer);
  firstInstructions->instruction = instruction;
  firstInstructions->prox = NULL;

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "loadI %d => rfp", (tabela == NULL) ? 1024 : tabela->accDesloc);
  instruction->inst = strdup(buffer);
  firstInstructions = addInstructionToList(firstInstructions, instruction);

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "loadI %d => rsp", (tabela == NULL) ? 1024 : tabela->accDesloc + STACK_FRAME_TAM_FIX + sInfo.nArgs * 4);
  instruction->inst = strdup(buffer);
  firstInstructions = addInstructionToList(firstInstructions, instruction);

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "storeAI rfp => rsp, 0");
  instruction->inst = strdup(buffer);
  firstInstructions = addInstructionToList(firstInstructions, instruction);

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "storeAI rpc => rsp, 4");
  instruction->inst = strdup(buffer);
  firstInstructions = addInstructionToList(firstInstructions, instruction);

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "i2i rsp => rfp");
  instruction->inst = strdup(buffer);
  firstInstructions = addInstructionToList(firstInstructions, instruction);

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "jumpI -> %s", main_label);
  instruction->inst = strdup(buffer);
  firstInstructions = addInstructionToList(firstInstructions, instruction);

  instruction = malloc(sizeof(ILOC_INST));
  sprintf(buffer, "halt");
  instruction->inst = strdup(buffer);
  firstInstructions = addInstructionToList(firstInstructions, instruction);

  return firstInstructions;
}

void genSaidaIloc(NODO_ARVORE* arvore, T_SIMBOLO* tabela){
  genNodeCode(arvore, tabela);

  ILOC_INST_LIST *outputList = genFirstInstructions(tabela);

  if(outputList == NULL)
    printf("Erro: Funcao \"main\" nao declarada\n");
  else{
    if(arvore != NULL){
      arvore->instructionList = concatInstructionLists(outputList, arvore->instructionList);
      outputList = arvore->instructionList;
    }

    fixPushPopRegisters(outputList);

    if(DEBUG_MODE){
      FILE *fp = fopen("saida.iloc", "w");
      printInstructionList(fp, outputList);
      fclose(fp);

      printf("Label Table:\n\n");
      print_label_table(label_table);

      printf("Codigo Gerado:\n\n");
    }
    printInstructionList(stdout, outputList);
  }

  free_label_table(label_table);

  if(DEBUG_MODE){
    optimize(outputList, 0);
    FILE *fp = fopen("saidaOpt.iloc", "w");
    printInstructionList(fp, outputList);
    fclose(fp);
  }
}
