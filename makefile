CFLAGS += -I./raylib/include -L./raylib/lib
LFLAGS += -l:libraylib.a -lm

snake: main.c makefile dl
	gcc $(CFLAGS) main.c -o snake $(LFLAGS)

dl:
	@echo "Downloading raylib..."
	@if [ ! -d "raylib" ]; then \
		./download-raylib.sh;\
		fi

run: snake
	./snake
clean:
	rm -f snake
	rm -rf raylib
