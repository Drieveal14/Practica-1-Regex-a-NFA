#ifndef REGEX_H
#define REGEX_H

#define CONCAT_OP '.'

typedef struct
{
    char value; /* literal o símbolo de operador (ver arriba) */
} regex_token;

typedef struct
{
    regex_token *items;
    int size;
    int capacity;
} regex;

/** Convierte una expresión regular infija a notación postfija. */
regex parse_regex(const char *input);

/** Libera el arreglo dinámico asociado a una expresión regular. */
void free_regex(regex *r);

#endif