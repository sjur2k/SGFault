#include "utils.h"

#include <string.h>
#include <stdbool.h>

bool str_eq(const char *str1, const char *str2){
    return strcmp(str1,str2)==0;
}