#ifndef SGFAULT_UTILS_H
#define SGFAULT_UTILS_H
#include <stdbool.h>

/* Probably unnecessary */
void *safe_malloc(size_t size);

/* Since strdup is not in versions prior to C23 */
char *str_dup(const char *s);

/* Helper function since strcmp is counter intuitive */
bool str_eq(const char *str1, const char *str2);

#endif