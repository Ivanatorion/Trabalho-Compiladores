all: etapa4

etapa4: bin/lex.yy.o bin/main.o bin/parser.tab.o bin/arvore.o bin/tabela.o
	gcc -o etapa4 bin/*.o -lfl

lex.yy.c: scanner.l
	flex scanner.l

bin/lex.yy.o: lex.yy.c parser.tab.c
	gcc -o bin/lex.yy.o -c lex.yy.c

parser.tab.c: parser.y include/defines.h
	bison -d parser.y

bin/parser.tab.o: parser.tab.c
	gcc -o bin/parser.tab.o -c parser.tab.c -Wall

bin/arvore.o: src/arvore.c include/defines.h
	gcc -o bin/arvore.o -c src/arvore.c -Wall

bin/tabela.o: src/tabela.c include/defines.h
	gcc -o bin/tabela.o -c src/tabela.c -Wall

bin/main.o: main.c
	gcc -o bin/main.o -c main.c -Wall

clean:
	rm -f etapa4 lex.yy.c main.o
	rm -rf bin/*.o
	rm -f parser.tab.c parser.tab.h
	rm -f e3.csv
