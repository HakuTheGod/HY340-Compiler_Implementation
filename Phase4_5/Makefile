rm:
	rm -rf calc
all:
	bison --yacc --defines -v --warnings=all --output=parser.c parser.y
	flex --outfile=al.c al.l
	gcc -o calc al.c parser.c