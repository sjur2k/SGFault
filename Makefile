ifeq ($(OS),Windows_NT)
	RMDIR := rmdir /S /Q
	RM := del /Q /F
	CP := copy
	MKDIR := mkdir
	EXE := .exe
	NEWLINE := .
	INSTALL_BIN := C:\\sgfault\\bin
	INSTALL_SHARE := C:\\sgfault\\share
	# S is short for Slash
	S := \\
else
	RMDIR := rm -rf
	RM := rm -f
	CP := cp
	MKDIR := mkdir -p
	EXE :=
	NEWLINE := 
	INSTALL_BIN := /usr/local/bin
	INSTALL_SHARE := /usr/local/share/
	# S is short for Slash
	S := /
endif

CC := gcc
CSTD := c99
CFLAGS = -std=$(CSTD) -Wall -Wextra -Wpedantic -Wstrict-prototypes -g -DDATA_DIR=\"$(INSTALL_SHARE)\"
SRC = src/main.c src/utils.c src/lexer.c src/compiler_args.c src/parser.c

COMPILER_BINARY = build/sgfault$(EXE)
FILE = testing/test.sg
NAME = $(basename $(notdir $(FILE)))

.PHONY: mkdir build install clean run

mkdir:
	@$(MKDIR) build

build: mkdir
	@echo $(NEWLINE)
	@echo 033[32mBuilding...\033[0m
	@$(CC) $(CFLAGS) -o $(COMPILER_BINARY) $(SRC) 

run: build # ONLY FOR TESTING
	@$(COMPILER_BINARY) -o $(dir $(FILE))$(NAME) $(FILE)
	@echo $(NEWLINE)
	@echo \033[32mRunning compiled file...\033[0m
ifeq ($(OS),Windows_NT)
	@$(dir $(FILE))$(NAME)$(EXE)
else
	@./$(dir $(FILE))$(NAME)$(EXE)
endif

install: mkdir
	@echo $(NEWLINE)
	@echo \033[32mInstalling SGFault ...\033[0m
	@$(CC) $(CFLAGS) -o $(COMPILER_BINARY) $(SRC)
ifeq ($(OS),Windows_NT)
	@$(CP) build\sgfault.exe $(INSTALL_BIN)\sgfault.exe
	@$(MKDIR) $(INSTALL_SHARE)\docs
	@$(CP) docs\* $(INSTALL_SHARE)\docs
else
	@$(CP) build/sgfault $(INSTALL_BIN)/sgfault
	@$(MKDIR) $(INSTALL_SHARE)/docs
	@$(CP) docs/* $(INSTALL_SHARE)/docs
endif
	@echo $(NEWLINE)
	@echo \033[32mInstallation sucessful!\n\033[0m

clean:
	@$(RMDIR) build
	@$(RM) testing/test$(EXE)

uninstall:
	@echo $(NEWLINE)
	@echo \033[32mUninstalling SGFault ...\033[0m
	@$(RMDIR) $(INSTALL_SHARE)
ifeq ($(OS),Windows_NT)
	@$(RM) $(INSTALL_BIN)\sgfault.exe
else
	@$(RM) $(INSTALL_BIN)/sgfault
endif
	@echo $(NEWLINE)
	@echo \033[32mUninstallation sucessful!\n\033[0m