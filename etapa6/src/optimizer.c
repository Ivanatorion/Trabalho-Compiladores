#include "../include/optimizer.h"

#include <string.h>
#include <stdlib.h>

char* getLabel(ILOC_INST* instruction){
  char* sepPos = strstr(instruction->inst, ":");

  if(sepPos == NULL)
    return NULL;

  int labelLen = (sepPos - instruction->inst);
  char* result = malloc(sizeof(char) * (labelLen + 1));

  for(int i = 0; i < labelLen; i++)
    result[i] = instruction->inst[i];

  result[labelLen] = '\0';

  return result;
}

int optNop(ILOC_INST_LIST *first, ILOC_INST_LIST *second){
  char *fi = first->instruction->inst;
  char *fs = second->instruction->inst;

  if(strstr(fi, "nop")){
    char *flabel = getLabel(first->instruction);
    char *slabel = getLabel(second->instruction);

    if(slabel == NULL){
        char buffer[128];

        sprintf(buffer, "%s: %s", flabel, fs);
        free(second->instruction->inst);
        free(first->instruction->inst);
        free(second->instruction);
        first->instruction->inst = strdup(buffer);

        first->prox = second->prox;
        free(second);

        free(flabel);
        return 1;
    }

    free(slabel);
    free(flabel);
  }

  return 0;
}

void optimize(ILOC_INST_LIST *instList, int optLevel){
  if(instList == NULL)
    return;

  ILOC_INST_LIST *first = instList;
  ILOC_INST_LIST *second = first->prox;
  int optAplied;

  while (second != NULL) {
    optAplied = 0;

    optAplied = optAplied + optNop(first, second);

    if(optAplied == 0){
      first = first->prox;
      second = first->prox;
    }
    else{
      second = first->prox;
    }
  }
}
