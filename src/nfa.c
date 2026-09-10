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
    nfa n;
    nfa_init(&n);

    frag stack[STACK_MAX];
    int top = -1;

    for (int i = 0; i < r.size; i++)
    {
        char c = r.items[i].value;

        if (c == CONCAT_OP)
        {
            frag f2 = stack[top--];
            frag f1 = stack[top--];
            add_trans(&n, f1.accept, NFA_EPSILON, f2.start);
            stack[++top] = (frag){f1.start, f2.accept};
        }
        else if (c == '|')
        {
            frag f2 = stack[top--];
            frag f1 = stack[top--];
            int s = new_state(&n);
            int a = new_state(&n);
            add_trans(&n, s, NFA_EPSILON, f1.start);
            add_trans(&n, s, NFA_EPSILON, f2.start);
            add_trans(&n, f1.accept, NFA_EPSILON, a);
            add_trans(&n, f2.accept, NFA_EPSILON, a);
            stack[++top] = (frag){s, a};
        }
        else if (c == '*')
        {
            frag f1 = stack[top--];
            int s = new_state(&n);
            int a = new_state(&n);
            add_trans(&n, s, NFA_EPSILON, f1.start);
            add_trans(&n, s, NFA_EPSILON, a);
            add_trans(&n, f1.accept, NFA_EPSILON, f1.start);
            add_trans(&n, f1.accept, NFA_EPSILON, a);
            stack[++top] = (frag){s, a};
        }
        else if (c == '+')
        {
            frag f1 = stack[top--];
            int a = new_state(&n);
            add_trans(&n, f1.accept, NFA_EPSILON, f1.start);
            add_trans(&n, f1.accept, NFA_EPSILON, a);
            stack[++top] = (frag){f1.start, a};
        }
        else if (c == '?')
        {
            frag f1 = stack[top--];
            int s = new_state(&n);
            int a = new_state(&n);
            add_trans(&n, s, NFA_EPSILON, f1.start);
            add_trans(&n, s, NFA_EPSILON, a);
            add_trans(&n, f1.accept, NFA_EPSILON, a);
            stack[++top] = (frag){s, a};
        }
        else
        {
            /* literal */
            int s = new_state(&n);
            int a = new_state(&n);
            add_trans(&n, s, c, a);
            stack[++top] = (frag){s, a};
        }
    }

    if (top >= 0)
    {
        frag f = stack[top--];
        n.start = f.start;
        n.accept = f.accept;
    }
    else
    {
        /* regex vacía: NFA que acepta solo la cadena vacía */
        int s = new_state(&n);
        n.start = s;
        n.accept = s;
    }

    return n;
}

/* ---------- simulación ---------- */
/* Simula la ejecución del NFA sobre una cadena de entrada. */

/* Agrega `state` y todos los estados alcanzables por epsilon a `set`,
 * marcando `visited` para evitar ciclos infinitos. */
static void add_epsilon_closure(nfa *n, int state, bool *set, bool *visited)
{
    if (visited[state])
        return;
    visited[state] = true;
    set[state] = true;

    nfa_state *s = &n->states[state];
    for (int i = 0; i < s->ntrans; i++)
    {
        if (s->trans[i].symbol == NFA_EPSILON)
        {
            add_epsilon_closure(n, s->trans[i].target, set, visited);
        }
    }
}

int match_nfa(nfa n, const char *str, int len)
{
    bool *current = calloc(n.size, sizeof(bool));
    bool *next = calloc(n.size, sizeof(bool));
    bool *visited = calloc(n.size, sizeof(bool));

    memset(visited, 0, n.size * sizeof(bool));
    add_epsilon_closure(&n, n.start, current, visited);

    for (int i = 0; i < len; i++)
    {
        memset(next, 0, n.size * sizeof(bool));
        memset(visited, 0, n.size * sizeof(bool));

        for (int s = 0; s < n.size; s++)
        {
            if (!current[s])
                continue;
            nfa_state *st = &n.states[s];
            for (int t = 0; t < st->ntrans; t++)
            {
                if (st->trans[t].symbol == str[i])
                {
                    add_epsilon_closure(&n, st->trans[t].target, next, visited);
                }
            }
        }

        memcpy(current, next, n.size * sizeof(bool));
    }

    int accepted = current[n.accept];

    free(current);
    free(next);
    free(visited);

    return accepted;
}


/* ---------- serialización ---------- */

bool save_nfa(nfa *n, const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f)
        return false;

    fprintf(f, "%d %d %d\n", n->size, n->start, n->accept);
    for (int i = 0; i < n->size; i++)
    {
        nfa_state *s = &n->states[i];
        fprintf(f, "%d", s->ntrans);
        for (int t = 0; t < s->ntrans; t++)
        {
            unsigned char sym = (unsigned char)s->trans[t].symbol;
            fprintf(f, " %d %d", sym, s->trans[t].target);
        }
        fprintf(f, "\n");
    }

    fclose(f);
    return true;
}