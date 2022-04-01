# We construct a separate target for the flex output file lex.yy.c, so that
# it can be compiled separately. Then in "all" you will combine all other
# code you might have into a single final executable.

all: lex.yy.c
	g++ lex.yy.c functions.cpp -std=c++17 -lfl -o shell

lex.yy.c: shell.l
	flex -o lex.yy.c shell.l

clean:
	rm -f lex.yy.c
	rm -f shell
