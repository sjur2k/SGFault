#include "utils.h"
#include <string.h>
#include <stdbool.h>

bool str_eq(char* str1, char* str2){
    return strcmp(str1,str2)==0;
}