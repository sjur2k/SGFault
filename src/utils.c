#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <windows.h>

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

const char *get_nasm_os(void){
    #ifdef _WIN32
        return "win64";
    #else
        return "elf64";
    #endif
}

const char *get_extension(void){
    #ifdef _WIN32
        return ".exe";
    #else
        return "";
    #endif
}

int run_command(const char *command){
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s", command);
    
    BOOL ok = CreateProcessA(NULL,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
    
    if(!ok){
        fprintf(stderr,"\033[1;31mError:\033[0;0m Process creation failed (%lu)\n",GetLastError());
        return -1;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return (int)exit_code;
}