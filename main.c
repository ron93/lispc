#include  "mpc.h"

#define MAXIMUM_BUFFER_SIZE 2048

#define LASSERT(args, cond, err) \
    if(!(cond)) { lval_del(args); return lval_err(err); }


/* compile this functions if on  windows*/
#ifdef _WIN32
#include <string.h>


static char buffer[MAXIMUM_SIZE]; /* bufer to hold user input of size 2048 */

/* fake readline function */
char* readline(char* prompt)
{
    fputs(promt, stdout);
    fgets(buffer, MAXIMUM_BUFFER_SIZE);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

/* fake history function */
void add_history(char* unused) {}

/* run this function otherwise*/
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

// foward declaration
struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;
void lval_print(lval* v);
lval* lval_add(lval* v, lval* x);
lval* lval_eval(lval* v);
lval* lval_take(lval* v, int i);
lval* lval_pop(lval* v, int i);
lval* builtin_op(lval* a, char* op);
lval* builtin(lval* a, char* func);

/* possible lval types*/
enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR };


typedef lval*(*lbuiltin)(lenv*, lval*);


/* to fix pointer or reference to incomplete type "struct lval" is not allowed
*/
// typedef struct cell;
/* lval(lispc values) type*/
typedef struct lval
{
    int type;
    long num;
    /* error and symbol have string data*/
    char* err;
    char* sym;
    lbuiltin fun;
    /* count */
    int count;
    lval** cell;
} lval;

// lenv struct
struct lenv {
    int count;
    char** syms;
    lval** vals;
};

/* possible errors*/
// enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

/* construct a pointer to a new number type lval*/
lval* lval_num(long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}
/* construct a pointer to a new error type lval*/
lval* lval_err(char* m) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(sizeof(m) + 1);
    strcpy(v->err, m);
    return v;
}

/* construct a pointer to a new Symbol lval*/
lval* lval_sym(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1); /* allocate space for symbol :strlen(s) + 1 -> to add space for the end of string char '\0'*/
    strcpy(v->sym, s);
    return v;
}

/*  A pointer to a new empty Sexpr lval*/
lval* lval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}
// pointer to new empty Qexpr lval
lval* lval_qexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}
// function constructor
lval* lval_fun(lbuiltin func) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->fun = func;
    return v;
}


lval* lval_read_num(mpc_ast_t* t) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}


lval* lval_read(mpc_ast_t* t) {
    /* check value and convert to target type */
    if (strstr(t->tag, "number")) { return lval_read_num(t); }
    if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

    /* if root(>) or sexpr create empty list */
    lval* x = NULL;
    if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
    if (strstr(t->tag, "sexpr")) { x = lval_sexpr(); }
    if (strstr(t->tag, "qexpr")) { x = lval_qexpr(); }

    /* fill list with valid expresion */
    for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "}") == 0 ) {continue; }
        if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
        if (strcmp(t->children[i]->tag, "regex") == 0) { continue; }
        x = lval_add(x, lval_read(t->children[i]));
    }
    return x;
}

lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count-1] = x;
    return v;
}

void lval_expr_print(lval* v, char open, char close) {
    putchar(open);
    for (int i = 0; i < v->count; i++) {
        /* print values within lval*/
        lval_print(v->cell[i]);

        /* don't print trailing space if it's the last element */
        if (i != (v->count-1)) {
            putchar(' ');
        }
    }
    putchar(close);
}

// delete logic
void lval_del(lval* v) {
    switch (v->type)
    {
        /* Do nothing for number type*/
        case LVAL_NUM: break;

        /* free string -> Err and Sym*/
        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;
        case LVAL_FUN: break;
        /* Sexpr and Qexpr handling ->delete if either*/
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            for (int i = 0; i < v->count; i++) {
                /* delete every element in an Sexpr i.e use recussion because every value of Sexpr can be either a Sym or Err*/
                lval_del(v->cell[i]);
            }
            /* free pointer memory */
            free(v->cell);
        break;
    }
    free(v);
}

/* print lval */
void lval_print(lval* v) {
    switch (v->type) {
        /* print if type is number*/
        case LVAL_NUM: printf("%li", v->num); break;

        /* error type */
        case LVAL_ERR: printf("Error: %s", v->err); break;
        case LVAL_SYM: printf("%s", v->sym); break;
        case LVAL_FUN: printf("<function>"); break;
        case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
        case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
    }
}

/* print lval followed by a newline */
void lval_println(lval* v) { lval_print(v); putchar('\n');}

lval* lval_eval_sexpr(lval* v) {
    /* evaluate children */
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(v->cell[i]);
    }
    /* error check */
    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type == LVAL_ERR) {return lval_take(v, i); }
    }
    /* empty Expression*/
    if (v->count == 0) { return v; }

    /* single Expression */
    if (v->count == 1) { return lval_take(v, 0); }


    /* Ensure first element is a Symbol */
    lval* f = lval_pop(v, 0);
    if (f->type != LVAL_SYM) {
        lval_del(f); lval_del(v);
        return lval_err("S-expression Does not start with symbol!");
    }

    /* Call builtin with operator */
    lval* result = builtin(v, f->sym);
    lval_del(f);
    return result;
}

lval* lval_eval(lval* v) {
    /* evaluate Sexpression */
    if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }

    /* all other lval types remain the same */
    return v;
}

lval* lval_pop(lval* v, int i) {
    // find the item at "i"
    lval* x = v->cell[i];

    // shift memory after the item at "i" over the top
    memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*) * (v->count-i-1));

    // decrease count of items in the list
    v->count--;

    // reallocate memory used 
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}

lval* lval_take(lval* v, int i) {
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}

lval* builtin_op(lval* a, char* op) {
    // ensure all args are numbers 
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type != LVAL_NUM) {
            lval_del(a);
            return lval_err("Cannot operate on non-number!");
        }
    }

    // pop first element
    lval* x = lval_pop(a, 0);

    // if no args and sub , perform unary negation
    if ((strcmp(op, "-") == 0) && a->count == 0) {
        x->num = -x->num;
    }

    // while elements stil in list
    while (a->count > 0) {
        // pop next element
        lval* y = lval_pop(a, 0);

        if (strcmp(op, "+") == 0) { x->num += y->num; }
        if (strcmp(op, "-") == 0) { x->num -= y->num; }
        if (strcmp(op, "*") == 0) { x->num *= y->num; }
        if (strcmp(op, "/") == 0) { 
            if (y->num == 0) {
                lval_del(x); lval_del(y);
                x = lval_err("Division by zero!"); break;
            }
            x->num /= y->num;
         }
         if (strcmp(op, "%") == 0) {
            if (y->num == 0) {
                lval_del(x); lval_del(y);
                x = lval_err("Division by zero!"); break;
            }
            x->num %= y->num;
         }
         lval_del(y);
    }
    lval_del(a); return x;
}

lval* builtin_head(lval* a) {
    // check error conditions
    LASSERT(a, a->count == 1, "Function 'head' passed too many arguments!");

    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'head' passed incorrect types!");

    LASSERT(a, a->cell[0]->count != 0,"Function 'head' passed {}!");

    // take first arg
    lval* v = lval_take(a, 0);

    // delete all elements that are not head and return
    while (v->count > 1) { lval_del(lval_pop(v, 1)); }
    return v;
}

lval* builtin_tail(lval* a) {
    // check Error conditions
    LASSERT(a, a->count == 1, "Functon 'tail' passed too many arguments!");

    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'tail' passed incorrect types!"); 

    LASSERT(a, a->cell[0]->count != 0, "Function 'tail' passed {}");

    // take first argument
    lval* v = lval_take(a, 0);

    // delete first argument and return
    lval_del(lval_pop(v, 0));
    return v;
}

// converts sexpr to qexpr
lval* builtin_list(lval* a) {
    a->type = LVAL_QEXPR;
    return a;
}

// takes single qexpr and converts to sexpr and eval using 'lval_eval'
lval* builtin_eval(lval* a) {
    LASSERT(a, a->count == 1, "Function 'eval' passed too many arguments!");
    LASSERT(a, a->cell[0]->type = LVAL_QEXPR, "Function 'eval' passed incorrect type!");

    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(x);
}

lval* lval_join(lval* x, lval* y) {
    // add every cell in 'y' to 'a'
    while (y->count) {
        x = lval_add(x, lval_pop(y, 0));
    }
    // delete the empty 'y' and return 'x'
    lval_del(y);
    return x;
}

// joins qexprs
lval* builtin_join(lval* a) {
    for (int i = 0; i , a->count; i++) {
        LASSERT(a, a->cell[i]->type == LVAL_QEXPR, "Function 'join' passed incorrect type.");
    }

    lval* x = lval_pop(a, 0);

    while (a->count) {
        x = lval_join(x, lval_pop(a, 0));
    }
    lval_del(a);
    return x;
    
}

// lookup the function to call
lval* builtin(lval* a, char* func) {
    if (strcmp("list", func) == 0) { return builtin_list(a); }
    if (strcmp("head", func) == 0) { return builtin_head(a); }
    if (strcmp("tail", func) == 0) { return builtin_tail(a); }
    if (strcmp("join", func) == 0) { return builtin_join(a); }
    if (strcmp("eval", func) == 0) { return builtin_eval(a); }
    if (strstr("+-/*%", func)) { return builtin_op(a, func); }
    lval_del(a);
    return lval_err("Unknown Function");
}

// function to copy lval
lval* lval_copy(lval* v) {
    lval* x = malloc(sizeof(lval));
    x->type = v->type;
    
    switch (v->type) {
    // copy directly -> for Numbers and Functions
    case LVAL_FUN: x->fun = v->fun; break;
    case LVAL_NUM: x->num = v->num; break;
    
    // copy Strings using malloc and strcpy
    case LVAL_ERR:
        x->err = malloc(strlen(v->err) + 1);
        strcpy(x->err, v->err); break;

    case LVAL_SYM:
        x->sym = malloc(strlen(v->sym) + 1);
        strcpy(x->sym, v->sym); break;

    // copy Lists by copying each sub-expression
    case LVAL_SEXPR:
    case LVAL_QEXPR:
        x->count = v->count;
        x->cell = malloc(sizeof(lval*) * x->count);
        for (int i = 0; i < x->count; i++) {
            x->cell[i] = lval_copy(v->cell[i]);
        }
    break;
    }
    return x;
}

int main(int arg, char**argv)
{
    /* Parsers*/
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Qexpr = mpc_new("qexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispc = mpc_new("lispc");

    /* define language*/
    mpca_lang(MPCA_LANG_DEFAULT, 
        "                                                                   \
            number : /-?[0-9]+/ ;                                           \
            symbol : /[a-zA-Z0-9_+%\\-*\\/\\\\=<>!&]+/;   \
            sexpr  : '(' <expr>* ')' ;                                      \
            qexpr  : '{' <expr>* '}' ;                                      \
            expr   : <number> | <symbol> | <sexpr> | <qexpr> ;              \
            lispc  : /^/ <expr>* /$/ ;                                      \
        ",
        Number, Symbol, Sexpr, Qexpr, Expr, Lispc);

    puts("lispc version 0.0.0.0.1"); /* version info */
    puts("Press ctrl+c to exit\n");

    while(1) 
    {
        char* input = readline("lispc> ");
        add_history(input); /*add input to history-> accessed by pressing the arrow key*/

        /* parse input*/
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispc, &r)) {
            /*  Load AST from output */
            mpc_ast_t* a = r.output;
            lval* x = lval_eval(lval_read(a));
            lval_println(x);
            lval_del(x);
        } 
        else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }
    mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispc);
    return 0;
}
