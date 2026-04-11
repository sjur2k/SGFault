#ifndef SGFAULT_PARSER_H
#define SGFAULT_PARSER_H

#include <stdbool.h>
#include "lexer.h"

/* void parse(TokenList *t_list, ParserContext *context); */

typedef struct{
    int line_number;
    bool has_error;
}ParserContext;

#endif