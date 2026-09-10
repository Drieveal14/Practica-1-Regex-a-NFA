#ifndef REGEX_H
#define REGEX_H

/*
 * Representa una expresion regular ya convertida a notación
 * postfija con concatenación explicita.
 */
#define CONCAT_OP '.'

typedef struct
{
    char value; /* literal o símbolo de operador (*, +, ?, |, .) */
} regex_token;

typedef struct
{
    regex_token *items;
    int size;
    int capacity;
} regex;

/* Convierte una expresión regular infija (con concatenación implícita)
 * en su representación postfija con concatenación explícita. */
regex parse_regex(const char *input);

/* Libera la memoria asociada a una regex. */
void free_regex(regex *r);

#endif