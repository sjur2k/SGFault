#ifndef SGFAULT_UTILS_H
#define SGFAULT_UTILS_H
#include <stdbool.h>

/* Probably unnecessary */
void *safe_malloc(size_t size);

/* Since strdup is not in versions prior to C23 */
char *str_dup(const char *s);

/* Helper function since strcmp is counter intuitive */
bool str_eq(const char *str1, const char *str2);

const char *get_data_dir(void);
const char *get_nasm_path(void);
const char *get_gcc_path(void);
const char *get_nasm_os(void);
const char *get_extension(void);

int run_command(const char *command);
#endif