all: etapa4

etapa4: lex.yy.c main.c parser.y src/arvore.c src/tabela.c
	bison -d parser.y
	gcc -c lex.yy.c parser.tab.c main.c src/arvore.c src/tabela.c
	gcc -o etapa4 arvore.o tabela.o lex.yy.o parser.tab.o main.o -lfl

lex.yy.c: scanner.l
	flex scanner.l

clean:
	rm -rf *.o *~ etapa4
	rm -rf lex.yy.*
	rm -rf parser.tab.*
