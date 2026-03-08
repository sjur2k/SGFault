CC = gcc
CFLAGS = -Wall -Wextra -g
SRC = src/sgfault.c

NAME = $(basename $(notdir $(FILE)))

COMPILER_BINARY = build/sgfault


.PHONY: build run execute clean all

build:
	@$(CC) $(CFLAGS) $(SRC) -o $(COMPILER_BINARY)

run: build
	@$(COMPILER_BINARY) $(FILE)
	@nasm -f elf64 $(NAME).asm -o $(NAME).o
	@ld $(NAME).o -o $(NAME)

execute:
	@$(NAME)
	@echo "\nProcess finished with exit code $$?"

all: build
	@$(COMPILER_BINARY) $(FILE)
	@nasm -f elf64 $(NAME).asm -o $(NAME).o
	@ld $(NAME).o -o $(NAME)
	@$(NAME)
	@echo "\nProcess finished with exit code $$?"
install: build
	@cp build/sgfault /usr/local/bin/sgfault
	@echo "Installed SGfault\n"
clean:
	@rm -f $(NAME) build/*.asm build/*.o

	