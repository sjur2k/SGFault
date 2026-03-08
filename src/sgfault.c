#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

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
}CompilerArgs;

CompilerArgs parse_args(int argc, char* argv[]){
    if(argc!=2){
        fprintf(stderr,"Correct usage is \"sgfault myFile.sg\"\n");
        exit(1);
    }

    char* file_name = strdup(argv[1]);
    char* slash = strrchr(file_name, '/');
    if (slash) {
        memmove(file_name,slash+1,strlen(slash));
    }
    char* extension = strrchr(file_name,'.');
    if (!extension || !str_eq(extension,".sg")){
        fprintf(stderr, "Error: expected a .sg file\n");
        exit(1);
    }
    *extension = '\0'; // Now file_name is terminated at the point
    

    FILE *in = fopen(argv[1], "r");
    if (!in) {
        perror("Error opening source file");
        exit(1);
    }
    
    char output_file[strlen(file_name)+11];
    snprintf(output_file,sizeof(output_file),"build/%s.asm",file_name);
    FILE *out = fopen(output_file,"w");
    if (!out) {
        perror("Error creating assembly file");
        exit(1);
    }
    
    CompilerArgs args = {argv[1],file_name, in, out};
    return args;
}
void compile(FILE *asm_file){
    //Write assembly file
    //PLACEHOLDER CODE:
    fprintf(asm_file,"global _start\n\n");
    fprintf(asm_file,"_start:\n");
    fprintf(asm_file,"\tmov rdi, 0\n");
    fprintf(asm_file,"\tmov rax, 60\n");
    fprintf(asm_file,"\tsyscall");

    //Tell system to assemble and link:
    char nasm_command[2*sizeof(asm_file)+36];
    printf("%d\n",(int)strlen(nasm_command));
    fflush(stdout);
    snprintf(nasm_command,sizeof(nasm_command),"nasm -f elf64 build/%s.asm -o build/%s.o",(char*)asm_file,(char*)asm_file);
}
int main(int argc, char *argv[]){
    CompilerArgs args = parse_args(argc,argv);
    compile(args.out);
    return 0;
}