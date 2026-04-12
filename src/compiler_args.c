#include "compiler_args.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include "utils.h"

#ifndef DATA_DIR
#define DATA_DIR "/usr/local/share/sgfault"
#endif

static void print_help(void){
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/docs/help.txt", DATA_DIR);
    FILE* help_file = fopen(path, "r");
    if(help_file == NULL){
        fprintf(stderr,"\033[1;31mError:\033[0m Couldn't open help file.\n");
        exit(1);
    }
    int symbol;
    while ((symbol = fgetc(help_file))!=EOF){
        putchar(symbol);
    }
    fclose(help_file);
}

CompilerArgs parse_args(int argc, char *argv[]){
    if(argc < 2 || argc > NUM_OPTIONS+2){
        fprintf(stderr,"Usage: sgfault <myFile.sg> [flags]\n");
        exit(1);
    }
    if(str_eq(argv[argc-1],"-h")||str_eq(argv[argc-1],"--help")){
        print_help();
        exit(0);
    };

    char *output_path = strdup(argv[argc-1]); // Uses name of src file unless set o-flag
    char *slash = strrchr(output_path, '/');
    if (slash) {
        memmove(output_path,slash+1,strlen(slash));
    }
    if (strlen(output_path)>100){
        fprintf(stderr, "\033[1;31mError:\033[0;0m use shorter filenames.\n");
        exit(1);
    }
    char *extension = strrchr(output_path,'.');
    if (!extension || !str_eq(extension,".sg")){
        fprintf(stderr, "\033[1;31mError:\033[0;0m expected a .sg file\n");
        fprintf(stderr,"Usage: sgfault <myFile.sg> [flags]\n");
        exit(1);
    }
    *extension = '\0';
    
    bool verbose = false;
    //In future check second condition against a list of valid flags.
    for (int i = 1; i < argc-1; i++){
        if (str_eq(argv[i],"--verbose") || str_eq(argv[i],"-v")){
            verbose = true;
        }else if(str_eq(argv[i],"--output") || str_eq(argv[i],"-o")){
            if (i+1 >= argc){
                fprintf(stderr,"\033[1;31mError:\033[0;0m --output (-o) requires a file path\n");
                exit(1);
            }
            free(output_path);
            output_path = strdup(argv[i+1]);
            i++;
        }else if(str_eq(argv[i],"--help") || str_eq(argv[i],"-h")){
            print_help();
            exit(0);
        }else{
            fprintf(stderr, "\033[1;31mError:\033[0;0m Undefined flag use\n");
            fprintf(stderr, "Usage: sgfault <my_file.sg> [flags]\n");
            fprintf(stderr, "Use --help for a list of valid flags\n");
            exit(1);
        }
    }

    FILE *source_file = fopen(argv[argc-1], "r");
    if (!source_file) {
        fprintf(stderr,"\033[1;31mError:\033[0;0m Couldn't open source file: %s\n",argv[argc-1]);
        exit(1);
    }
    
    char output_path_asm[PATH_MAX];
    snprintf(output_path_asm,sizeof(output_path_asm),"%s.asm",output_path);
    FILE *out = fopen(output_path_asm,"w");
    if (!out) {
        fprintf(stderr,"\033[1;31mError:\033[0;0m Couldn't create assembly file: %s\n",output_path_asm);
        exit(1);
    }
    CompilerArgs args = {
        .output_path    = output_path,
        .source_file    = source_file,
        .out            = out,
        .verbose        = verbose,
    };
    return args;
}

void build_binary(CompilerArgs *args){
    
    //Tell system to assemble and link:
    char nasm_command[2*PATH_MAX + 23]; // This is a bit pedantic, but helps me build understanding
    char linker_command[2*PATH_MAX + 8];
    char cleanup_command[2*PATH_MAX + 12];
    
    snprintf(nasm_command, sizeof(nasm_command), "nasm -f elf64 %s.asm -o %s.o", args->output_path, args->output_path);
    snprintf(linker_command, sizeof(linker_command), "ld %s.o -o %s", args->output_path, args->output_path);
    snprintf(cleanup_command, sizeof(cleanup_command), "rm -f %s.o %s.asm", args->output_path, args->output_path);

    if (system(nasm_command) != 0){
        system(cleanup_command);
        fprintf(stderr, "\033[1;31mError:\033[0;0m assembly failed\n");
        exit(1);
    }
    if (system(linker_command) != 0){
        system(cleanup_command);
        fprintf(stderr,"\033[1;31mError:\033[0;0m linking failed\n");
        exit(1);
    }
    if (!args->verbose){
        system(cleanup_command);
    }
}

void compiler_args_free(CompilerArgs *args){
    fclose(args->source_file);
    free(args->output_path);
}