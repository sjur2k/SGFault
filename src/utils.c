#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "utils.h"

#ifndef SGFAULT_DIR
    #ifdef _WIN32
        #define SGFAULT_DIR "C:\\sjur2k\\SGFault" // Specifically for the work pc. TODO: Generalize
        #define NASM_PATH SGFAULT_DIR "\\tools\\windows\\mingw64\\bin\\nasm.exe"
        #define GCC_PATH SGFAULT_DIR "\\tools\\windows\\mingw64\\bin\\gcc.exe" 
    #else
        #define SGFAULT_DIR "/usr/local/share/sgfault"
        #define NASM_PATH SGFAULT_DIR "/tools/linux/nasm"
        #define GCC_PATH SGFAULT_DIR "/tools/linux/gcc"
    #endif
#endif

void *safe_malloc(size_t size){
    void *ptr = malloc(size);
    if(!ptr){
        fprintf(stderr,"\033[1;31mOut of memory\n\033[0m");
        exit(1);
    }
    return ptr;
}

char *str_dup(const char *s){
    char *copy = safe_malloc(strlen(s) + 1);
    strcpy(copy, s);
    return copy;
}

bool str_eq(const char *str1, const char *str2){
    return strcmp(str1,str2)==0;
}

const char *get_data_dir(void){
    return SGFAULT_DIR;
}

const char *get_nasm_path(void){
    return NASM_PATH;
}

const char *get_gcc_path(void){
    return GCC_PATH;
}