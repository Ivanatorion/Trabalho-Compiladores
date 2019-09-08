all: etapa2

etapa2: lex.yy.c main.c parser.y
	bison -d parser.y
	gcc -c lex.yy.c parser.tab.c main.c
	gcc -o etapa2 lex.yy.o parser.tab.o main.o -lfl

lex.yy.c: scanner.l
	flex scanner.l

etapa2.tgz: main.c parser.y scanner.l Makefile
	tar czf etapa2.tgz main.c parser.y scanner.l Makefile

clean:
	rm -rf *.o *~ etapa2
	rm -rf lex.yy.*
	rm -rf parser.tab.*
