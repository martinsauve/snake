CFLAGS += -I./raylib/include -L./raylib/lib
LFLAGS += -l:libraylib.a -lm

snake: main.c makefile dl
	gcc $(CFLAGS) main.c -o snake $(LFLAGS) && ./snake

dl:
	@echo "Downloading raylib..."
	@if [ ! -d "raylib" ]; then \
		./download-raylib.sh;\
		fi

clean:
	rm -f snake
	rm -rf raylib
