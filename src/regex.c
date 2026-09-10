#include "regex.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---------- utilidades de arreglo dinámico de regex_token ---------- */

/* Inicializa una regex vacía con capacidad para crecer dinámicamente. */
static void regex_init(regex *r)
{
    r->size = 0;
    r->capacity = 16;
    r->items = malloc(sizeof(regex_token) * r->capacity);
}



void free_regex(regex *r)
{
    if (r->items)
        free(r->items);
    r->items = NULL;
    r->size = 0;
    r->capacity = 0;
}

/* ---------- paso 1: concatenación implícita -> explícita ---------- */

/* Indica si el carácter puede iniciar un operando o grupo. */
static int starts_operand(char c)
{
    return c == '(' || (c != ')' && c != '|' && c != '*' &&
                         c != '+' && c != '?' && c != CONCAT_OP);
}

/* Indica si el carácter puede terminar un operando o grupo. */
static int ends_operand(char c)
{
    return c == ')' || c == '*' || c == '+' || c == '?' ||
           (c != '(' && c != '|' && c != CONCAT_OP);
}


/* Inserta CONCAT_OP entre operandos adyacentes y conserva los escapes.
 * El resultado es una copia que el llamador debe liberar con free(). */
static char *make_explicit(const char *input)
{
    size_t len = strlen(input);
    char *out = malloc((len * 2 + 1) * sizeof(char)); 
    size_t j = 0;

    int prev_ends_operand = 0; /* ¿el último token emitido puede cerrar un operando? */

    for (size_t i = 0; i < len; i++)
    {
        char c = input[i];
        int escaped = 0;
        char literal = c;

        if (c == '\\' && i + 1 < len)
        {
            escaped = 1;
            i++;
            literal = input[i];
        }

        char effective = escaped ? 'a' /* cualquier literal se comporta igual */ : c;

        if (prev_ends_operand && starts_operand(effective) && effective != CONCAT_OP)
        {
            out[j++] = CONCAT_OP;
        }

        if (escaped)
        {
            out[j++] = '\\';
            out[j++] = literal;
            prev_ends_operand = 1;
        }
        else
        {
            out[j++] = c;
            prev_ends_operand = ends_operand(c);
        }
    }

    out[j] = '\0';
    return out;
    
}


/* ---------- paso 2: shunting-yard (infijo explícito -> postfijo) ---------- */

