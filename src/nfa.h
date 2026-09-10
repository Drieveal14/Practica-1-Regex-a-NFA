#ifndef NFA_H
#define NFA_H

#include <stdbool.h>
#include "regex.h"

#define NFA_EPSILON '\$' /* símbolo especial que marca una transición epsilon */

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

nfa regex_to_nfa(regex r);

int match_nfa(nfa n, const char *str, int len);

void free_nfa(nfa *n);

bool save_nfa(nfa *n, const char *path);

#endif