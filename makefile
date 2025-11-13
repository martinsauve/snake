snake: main.c makefile
	gcc -O3 -lraylib -lm main.c -o snake && ./snake
