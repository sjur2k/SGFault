#include "compiler_args.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

CompilerArgs parse_args(int argc, char* argv[]){
    if(argc < 2 || argc > NUM_FLAGS+2){
        fprintf(stderr,"Usage: sgfault <myFile.sg> [flags]\n");
        exit(1);
    }

    char* file_name = strdup(argv[1]);
    char* slash = strrchr(file_name, '/');
    if (slash) {
        memmove(file_name,slash+1,strlen(slash));
    }
    if (strlen(file_name)>100){
        fprintf(stderr, "Error: use shorter filenames.\n");
        exit(1);
    }
    char* extension = strrchr(file_name,'.');
    if (!extension || !str_eq(extension,".sg")){
        fprintf(stderr, "Error: expected a .sg file\n");
        fprintf(stderr,"Usage: sgfault <myFile.sg> [flags]\n");
        exit(1);
    }
    *extension = '\0'; // Now file_name is terminated at the point
    
    bool verbose = false;
    char* output_dir = ".";
    //In future check second condition against a list of valid flags.
    for (int i = 2; i < argc; i++){
        if (str_eq(argv[i],"--verbose") || str_eq(argv[i],"-v")){
            verbose = true;
        }else if(str_eq(argv[i],"--output") || str_eq(argv[i],"-o")){
            if (i+1 >= argc){
                fprintf(stderr,"Error: --output (-o) requires a file name\n");
                exit(1);
            }
            file_name = argv[i+1];
            i++;
        }else if(str_eq(argv[i],"--output-dir")||str_eq(argv[i],"-d")){
            if (i+1 >= argc){
                fprintf(stderr,"Error: --output-dir (-d) requires a directory path\n");
                exit(1);
            }
            output_dir = argv[i+1];
            i++;
        }else if(str_eq(argv[i],"--help") || str_eq(argv[i],"-h")){
            system("cat docs/help.txt");
            exit(0);
        }else{
            fprintf(stderr, "Error: Undefined flag use");
            fprintf(stderr, "Usage: sgfault <my_file.sg> [flags]\n");
            fprintf(stderr, "Use --help for a list of valid flags");
            exit(1);
        }
    }

    FILE *in = fopen(argv[1], "r");
    if (!in) {
        perror("Error opening source file");
        exit(1);
    }
    
    char output_file[255];
    snprintf(output_file,sizeof(output_file),"%s/%s.asm",output_dir,file_name);
    FILE *out = fopen(output_file,"w");
    if (!out) {
        perror("Error creating assembly file");
        exit(1);
    }
    
    CompilerArgs args = {argv[1], file_name, output_dir, in, out, verbose};
    return args;
}

void compilation_commands(CompilerArgs* args){
    
    //Tell system to assemble and link:
    char output_path[256];
    char nasm_command[1024];
    char linker_command[1024];
    char cleanup_command[1024]; // Bufsize might be a bit overkill
    
    snprintf(output_path, sizeof(output_path), "%s/%s", args->output_dir, args->output_name);
    snprintf(nasm_command, sizeof(nasm_command), "nasm -f elf64 %s.asm -o %s.o", output_path, output_path);
    snprintf(linker_command, sizeof(linker_command), "ld %s.o -o %s", output_path, output_path);
    snprintf(cleanup_command, sizeof(cleanup_command), "rm -f %s.o %s.asm", output_path, output_path);

    if (system(nasm_command) != 0){
        system(cleanup_command);
        fprintf(stderr, "Error: assembly failed\n");
        exit(1);
    }
    if (system(linker_command) != 0){
        system(cleanup_command);
        fprintf(stderr,"Error: linking failed\n");
        exit(1);
    }
    if (!args->verbose){
        system(cleanup_command);
    }
}
