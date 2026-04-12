#include <string.h>
#include <stdbool.h>

#include "utils.h"

bool str_eq(const char *str1, const char *str2){
    return strcmp(str1,str2)==0;
}