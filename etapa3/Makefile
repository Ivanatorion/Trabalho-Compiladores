all: etapa3

etapa3: lex.yy.c main.c parser.y src/arvore.c
	bison -d parser.y
	gcc -c lex.yy.c parser.tab.c main.c src/arvore.c
	gcc -o etapa3 arvore.o lex.yy.o parser.tab.o main.o -lfl

lex.yy.c: scanner.l
	flex scanner.l

etapa3.tgz: main.c parser.y scanner.l Makefile
	tar czf etapa3.tgz main.c parser.y scanner.l Makefile

clean:
	rm -rf *.o *~ etapa3
	rm -rf lex.yy.*
	rm -rf parser.tab.*
	rm -rf e3.csv
