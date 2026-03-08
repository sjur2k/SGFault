run:
	gcc src/main.c -o build/main
	./build/main
	nasm -f elf64 build/output.asm -o build/output.o
	ld build/output.o -o build/output
	./build/output