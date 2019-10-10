#include <stdio.h>
#include "parser.tab.h" //arquivo gerado com bison -d parser.y
#include "include/tabela.h"

extern int yylex_destroy(void);

void *arvore = NULL;
T_SIMBOLO* tabelaSimbolos;
void libera (void *arvore);
void exporta (void *arvore);

int main (int argc, char **argv)
{
  tabelaSimbolos = make_tabela();
  int ret = yyparse();
  exporta (arvore);
  libera(arvore);
  arvore = NULL;
  yylex_destroy();
  return ret;
}
