all: linux

build-dir:
	mkdir -p ./build

linux: build-dir
	gcc 2048.c -o ./build/2048 -lm

linux32: build-dir
	gcc -m32 2048.c -o ./build/2048.64 -lm

win: build-dir
	i586-mingw32msvc-gcc 2048.c -o ./build/2048.exe
