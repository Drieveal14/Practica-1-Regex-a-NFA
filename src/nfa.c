#include "nfa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fragmento de NFA con un único estado inicial y un único estado de
 * aceptación (invariante de la construcción de Thompson). */
typedef struct
{
    int start;
    int accept;
} frag;

/* ---------- manejo del arreglo dinámico de estados ---------- */

static void nfa_init(nfa *n)
{
    n->size = 0;
    n->capacity = 16;
    n->states = malloc(sizeof(nfa_state) * n->capacity);
    n->start = -1;
    n->accept = -1;
}

/* Reserva un nuevo estado y devuelve su índice dentro del NFA. */
static int new_state(nfa *n)
{
    if (n->size == n->capacity)
    {
        n->capacity *= 2;
        n->states = realloc(n->states, sizeof(nfa_state) * n->capacity);
    }
    n->states[n->size].ntrans = 0;
    n->states[n->size].trans[0].target = -1;
    n->states[n->size].trans[1].target = -1;
    return n->size++;
    
}

/* Añade una transición; Thompson garantiza que hay espacio para dos. */
static void add_trans(nfa *n, int from, char symbol, int to)
{
    nfa_state *s = &n->states[from];
    s->trans[s->ntrans].symbol = symbol;
    s->trans[s->ntrans].target = to;
    s->ntrans++;
 }   

void free_nfa(nfa *n)
{
    if (n->states)
        free(n->states);
    n->states = NULL;
    n->size = 0;
    n->capacity = 0;
    n->start = -1;
    n->accept = -1;
}


/* ---------- construcción de Thompson a partir de la postfija ---------- */

#define STACK_MAX 2048

nfa regex_to_nfa(regex r)
{
    
}

/* ---------- simulación ---------- */
/* Simula la ejecución del NFA sobre una cadena de entrada. */





/*añade la cerradura epsilon de un estado*/
static void add_epsilon_closure(nfa *n, int state, bool *set, bool *visited)
{

}

/* Determina si una cadena es aceptada por el NFA. */
int match_nfa(nfa n, const char *str, int len)
{
   
}



/* ---------- representación ---------- */
/* Representa  el NFA a un archivo de texto plano en `path`
*/
 
bool save_nfa(nfa *n, const char *path)
{
    
}