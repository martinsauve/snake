CFLAGS += -I./raylib/include -L./raylib/lib -Wall -Wextra -Wpedantic -O3
STATIC_LFLAGS += -l:libraylib.a -lm
LFLAGS += -lraylib -lm

#static: main.c makefile dl
#	gcc $(CFLAGS) main.c -o snake $(STATIC_LFLAGS)

snake: main.c makefile dl
	gcc $(CFLAGS) main.c -o snake $(STATIC_LFLAGS)

dl: download-raylib.sh
	@if [ ! -d "raylib" ]; then \
		./download-raylib.sh;\
		fi

run: snake
	./snake
clean:
	rm -f snake
	rm -rf raylib
