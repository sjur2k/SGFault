#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void initAsm(FILE* asmFile){
    fprintf(asmFile,"global _start\n\n");
    fprintf(asmFile,"_start:\n");
    fprintf(asmFile,"\tmov rdi, 0\n");
    fprintf(asmFile,"\tmov rax, 60\n");
    fprintf(asmFile,"\tsyscall");
    return;
}

void parse(FILE* sgFile){
    int c = 0;
    while((c=fgetc(sgFile))!= EOF){
        printf("%c",c);
        fflush(stdout); 
    }
}

int main(){
    FILE *out = fopen("build/output.asm", "w");
    FILE *in = fopen("src/main.sg","r");
    if (in == NULL || out == NULL) { 
        perror("fopen failed"); return 1;
    }
    initAsm(out);
    parse(in);
    return 0;
}