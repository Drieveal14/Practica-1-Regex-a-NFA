#include "regex.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---------- funciones para el arreglo dinámico de regex_token ---------- */

static void regex_init(regex *r)
{
    r->size = 0;
    r->capacity = 16;
    r->items = malloc(sizeof(regex_token) * r->capacity);
}

static void regex_push(regex *r, char value)
{
    if (r->size >= r->capacity)
    {
        r->capacity *= 2;
        r->items = realloc(r->items, sizeof(regex_token) * r->capacity);
    }
    r->items[r->size].value = value;
    r->size++;
}

void free_regex(regex *r)
{
    if (r->items)
        free(r->items);
    r->items = NULL;
    r->size = 0;
    r->capacity = 0;
}

/* ---------- paso 1: hacemos la concatenacion explícita ---------- */

/*Caracteres que puede actuar como final de un operando/grupo*/
static int ends_operand(char c)
{
    return c == ')' || c == '*' || c == '+' || c == '?' ||
           (c != '(' && c != '|' && c != CONCAT_OP);
}

/*Caracteres que puede actuar como inicio de un operando/grupo*/
static int starts_operand(char c)
{
    return c == '(' || (c != ')' && c != '|' && c != '*' &&
                         c != '+' && c != '?' && c != CONCAT_OP);
}

/* Devuelve una copia de la entrada haciendo explicita la concatenación 
 * (CONCAT_OP)  */
static char *make_explicit(const char *input)
{
    size_t len = strlen(input);
    /* en el peor de los casos se duplica el tamaño (un CONCAT_OP entre
     * cada par de caracteres) */
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

static int precedence(char op)
{
    switch (op)
    {
        case '*':
        case '+':
        case '?':
            return 3;
        case CONCAT_OP:
            return 2;
        case '|':
            return 1;
        default:
            return 0;
    }
}

static int is_unary(char op)
{
    return op == '*' || op == '+' || op == '?';
}

static int is_operator(char c)
{
    return c == '*' || c == '+' || c == '?' || c == '|' || c == CONCAT_OP;
}

regex parse_regex(const char *input)
{
    regex r;
    regex_init(&r);

    char *expr = make_explicit(input);
    size_t len = strlen(expr);

    char *op_stack = malloc((len + 1) * sizeof(char));
    int top = -1; /* indce del tope de la pila de ops. */

    for (size_t i = 0; i < len; i++)
    {
        char c = expr[i];

        if (c == '\\' && i + 1 < len)
        {
            
            regex_push(&r, expr[i + 1]);
            i++;
            continue;
        }

        if (c == '(')
        {
            op_stack[++top] = c;
        }
        else if (c == ')')
        {
            while (top >= 0 && op_stack[top] != '(')
            {
                regex_push(&r, op_stack[top--]);
            }
            if (top >= 0)
                top--; /* descarta el '(' */
        }
        else if (is_operator(c))
        {
            while (top >= 0 && op_stack[top] != '(' &&
                   precedence(op_stack[top]) >= precedence(c))
            {
                regex_push(&r, op_stack[top--]);
            }
            op_stack[++top] = c;
        }
        else
        {
            /* literal normal */
            regex_push(&r, c);
        }
    }

    while (top >= 0)
    {
        regex_push(&r, op_stack[top--]);
    }

    free(op_stack);
    free(expr);
    (void)is_unary;

    return r;
}
