CFLAGS += -I./raylib/include -L./raylib/lib -Wall -Wextra -Wpedantic -O3
LFLAGS += -l:libraylib.a -lm

snake: main.c makefile dl
	gcc $(CFLAGS) main.c -o snake $(LFLAGS)

dl: download-raylib.sh
	@if [ ! -d "raylib" ]; then \
		./download-raylib.sh;\
		fi

run: snake
	./snake
clean:
	rm -f snake
	rm -rf raylib
