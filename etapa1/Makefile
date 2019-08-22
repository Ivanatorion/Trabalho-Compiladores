all: etapa1

etapa1: lex.yy.c main.c tokens.h
	gcc -o etapa1 main.c lex.yy.c -lfl

lex.yy.c: scanner.l
	flex scanner.l

etapa1.tgz: main.c tokens.h scanner.l Makefile
	tar czf etapa1.tgz main.c tokens.h scanner.l Makefile

clean:
	rm -rf *.o *~ etapa1
	rm lex.yy.c
