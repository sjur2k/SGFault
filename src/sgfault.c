#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#define NUM_FLAGS 2

/* void initAsm(FILE* asmFile){
    fprintf(asmFile,"global _start\n\n");
    fprintf(asmFile,"_start:\n");
    fprintf(asmFile,"\tmov rdi, 0\n");
    fprintf(asmFile,"\tmov rax, 60\n");
    fprintf(asmFile,"\tsyscall");
    return;
} */

/* void parse(FILE* sgFile){
    int c = 0;
    while((c=fgetc(sgFile))!= EOF){
        printf("%c",c);
        fflush(stdout);
    }
} */

// Helper function since strcmp is counter intuitive
bool str_eq(char* str1, char* str2){
    return strcmp(str1,str2)==0;
}

typedef struct CompilerArgs{
    char *source_file;
    char *output_name;
    FILE *in;
    FILE *out;
    bool verbose;
}CompilerArgs;

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
    //In future check second condition against a list or hash table of valid flags.
    for (int i = 2; i < argc; i++){
        if (str_eq(argv[i],"--verbose") || str_eq(argv[i],"-v")){
            verbose = true;
        }else if(str_eq(argv[i],"--output") || str_eq(argv[i],"-o")){
            if (i+1 >= argc){
                fprintf(stderr,"Error: -o requires a file name\n");
                exit(1);
            }
            file_name = argv[i+1];
        }else if(str_eq(argv[i],"--help") || str_eq(argv[i],"-h")){
            system("cat docs/help.txt");
            exit(0);
        }else{
            fprintf(stderr, "Error: Undefined flag use");
            fprintf(stderr, "Usage: sgfault <myFile.sg> [flags]\n");
            fprintf(stderr, "Use --help for a list of valid flags");
            exit(1);
        }

    FILE *in = fopen(argv[1], "r");
    if (!in) {
        perror("Error opening source file");
        exit(1);
    }
    
    char output_file[255];
    snprintf(output_file,sizeof(output_file),"%s.asm",file_name);
    FILE *out = fopen(output_file,"w");
    if (!out) {
        perror("Error creating assembly file");
        exit(1);
    }
    
    CompilerArgs args = {argv[1],file_name, in, out};
    return args;
}
void compile(CompilerArgs *args){
    //Write assembly file
    //PLACEHOLDER CODE:
    fprintf(args->out,"global _start\n\n");
    fprintf(args->out,"_start:\n");
    fprintf(args->out,"\tmov rdi, 0\n");
    fprintf(args->out,"\tmov rax, 60\n");
    fprintf(args->out,"\tsyscall");

    //Tell system to assemble and link:
    char nasm_command[256];
    char linker_command[256];
    char cleanup_command[256];
    snprintf(nasm_command,sizeof(nasm_command),"nasm -f elf64 %s.asm -o %s.o",args->output_name,args->output_name);
    snprintf(linker_command,sizeof(linker_command),"ld %s.o -o %s",args->output_name,args->output_name);
    
    system(nasm_command);
    system(linker_command);

    system(cleanup_command);
}
int main(int argc, char *argv[]){
    CompilerArgs args = parse_args(argc,argv);
    compile(args.out,args.output_name);
    return 0;
}