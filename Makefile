ifeq ($(OS),Windows_NT)
	RMDIR := rmdir /S /Q
	RM := del /Q /F
	CP := copy
	MKDIR := mkdir
	EXE := .exe
	INSTALL_BIN := C:\\sgfault\\bin
	INSTALL_SHARE := C:\\sgfault\\share
	# S is short for Slash
	S := \\
	RUNEXE := 
else
	RMDIR := rm -rf
	RM := rm -f
	CP := cp
	MKDIR := mkdir -p
	EXE :=
	INSTALL_BIN := /usr/local/bin
	INSTALL_SHARE := /usr/local/share
	S := /
	RUNEXE = ./
endif

CC := gcc
CSTD := c99
CFLAGS = -std=$(CSTD) -Wall -Wextra -Wpedantic -Wstrict-prototypes -g '-DDATA_DIR="$(INSTALL_SHARE)"'
SRC = src/main.c src/utils.c src/lexer.c src/compiler_args.c src/parser.c

COMPILER_BINARY = build/sgfault$(EXE)
FILE = testing/test.sg
NAME = $(basename $(notdir $(FILE)))

.PHONY: mkdir build install clean run uninstall

mkdir:
	-@$(MKDIR) build

build: mkdir
	@printf '\033[32mBuilding...\033[0m\n'
	@$(CC) $(CFLAGS) -o $(COMPILER_BINARY) $(SRC) 

run: build # ONLY FOR TESTING
	@$(COMPILER_BINARY) -o $(dir $(FILE))$(NAME) $(FILE)
	@printf '\033[32mRunning compiled file...\033[0m\n'
	@$(RUNEXE)$(dir $(FILE))$(NAME)$(EXE)

install: mkdir
	@printf '\033[32mInstalling SGFault ...\033[0m\n'
	@$(CC) $(CFLAGS) -o $(COMPILER_BINARY) $(SRC)
	@$(CP) build$(S)sgfault$(EXE) $(INSTALL_BIN)$(S)sgfault$(EXE)
	-@$(MKDIR) $(INSTALL_SHARE)$(S)docs
	@$(CP) docs$(S)* $(INSTALL_SHARE)$(S)docs
	@printf '\033[32mInstallation successful!\033[0m\n'

clean:
	@$(RMDIR) build
	@$(RM) testing$(S)test$(EXE)

uninstall:
	@printf '\033[32mUninstalling SGFault ...\033[0m\n'
	@$(RMDIR) $(INSTALL_SHARE)
	@$(RM) $(INSTALL_BIN)$(S)sgfault$(EXE)
	@printf '\033[32mUninstallation successful!\033[0m\n'