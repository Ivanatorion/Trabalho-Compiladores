#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "iloc.h"

//Remove nops
int optNop(ILOC_INST_LIST *first, ILOC_INST_LIST *second);

void optimize(ILOC_INST_LIST *instList, int optLevel);

#endif
