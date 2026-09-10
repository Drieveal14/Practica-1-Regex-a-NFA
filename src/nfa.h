#ifndef NFA_H
#define NFA_H

#include <stdbool.h>
#include "regex.h"

#define NFA_EPSILON '\$' /* símbolo especial que marca una transición epsilon */

/* Cada estado puede tener hasta 2 transiciones (construcción de
 * Thompson: un estado nunca necesita más de dos transiciones salientes,
 * ya sean dos epsilon o una transición por símbolo). */
 
typedef struct
{
    char symbol;  /* NFA_EPSILON para transiciones epsilon */
    int target;   /* índice del estado destino, o -1 si no existe */
} nfa_transition;

typedef struct
{
    nfa_transition trans[2];
    int ntrans;
} nfa_state;

typedef struct
{
    nfa_state *states;
    int size;
    int capacity;
    int start;
    int accept;
} nfa;

/* Construye un NFA a partir de una regex en notación postfija usando
 * el algoritmo de construcción de Thompson. */

nfa regex_to_nfa(regex r);

/* Simula el NFA sobre `str` y regresa 1 si la
 * cadena es aceptada, 0 en caso contrario. */

int match_nfa(nfa n, const char *str, int len);

/* Libera la memoria asociada al NFA. */

void free_nfa(nfa *n);

/* Serializa el NFA a un archivo de texto plano en `path`. */

bool save_nfa(nfa *n, const char *path);

#endif