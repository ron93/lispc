#include  "mpc.h"

#define MAXIMUM_BUFFER_SIZE 2048
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

/* lval(lispc values) type*/
typedef struct
{
    int type;
    long num;
    /* error and symbol have string data*/
    char* err;
    char* sym;
    /* count to aa list of "lval*" */
    int count;
    /* Pointer to a list of "lval*" */
    struct  lval** cell;
} lval;

/* possible lval types*/
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };

/* possible errors*/
// enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

/* create a pointer to a new number type lval*/
lval* lval_num(long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}
/* create a pointer to a new error type lval*/
lval* lval_err(int x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = x;
    return v;
}

/* create a pointer to a new Symbol lval*/
lval* lval_sym(char * s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}

/*  */

/* print lval */
void lval_print(lval v) {
    switch (v.type) {
        /* print if type is number*/
        case LVAL_NUM: printf("%li", v.num); break;

        /* error type */
        case LVAL_ERR:
            /* check type of error and print */
            if (v.err == LERR_DIV_ZERO) {
                printf("Error: Division by Zero!");
            }   
            if (v.err == LERR_BAD_OP) {
                printf("Error: Invalid Operator!");
            }
            if (v.err == LERR_BAD_NUM) {
                printf("Error: Invalid Number!");
            }
        break;
    }
}

/* print lval followed by a newline */
void lval_println(lval v) { lval_print(v); putchar('\n');}

/* use operator string to see which operation to perform */
lval eval_op(lval x, char* op, lval y) {

    /* check if value id error and return it */
    if (x.type == LVAL_ERR) { return x; }
    if (y.type == LVAL_ERR) { return y; }

    if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
    if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
    if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
    if (strcmp(op, "/") == 0) { 
    /* if second operand is 0 return error*/
        return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num); 
    }
    return lval_err(LERR_BAD_OP);
}
lval eval(mpc_ast_t* t) {
    /* if tagged as a number return it directly */
    if (strstr(t->tag, "number")) {
        /* check for error in conversion */
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }
    /* the operator is always the second child */
    char* op = t-> children[1]->contents;
    /* store 3rd child in x */
    lval x = eval(t-> children[2]);

    /* iterate the remaining children and combining */
    int i = 3;
    while (strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }
    return x;
}



int main(int arg, char**argv)
{
    /* Parsers*/
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispc = mpc_new("lispc");

    /* define language*/
    mpca_lang(MPCA_LANG_DEFAULT, 
        "                                                     \
            number   : /-?[0-9]+/ ;                             \
            symbol   : '+' | '-' | '*' | '/' ;                  \
            sexpr    : '(' <expr>* ')' ; \
            expr     : <number> | symbol | <sexpr> ;  \
            lispc    : /^/ <expr>+ /$/ ;             \
        ",
        Number, Symbol, Sexpr, Expr, Lispc);

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
            // printf("Tag: %s\n", a->tag);
            // printf("Contents: %s\n", a->contents);
            // printf("NUmber of children: %i\n", a->children_num);

            // /* Get first child */
            // mpc_ast_t* c0 = a->children[0];
            // printf("First child Tag: %s\n", c0->tag);
            // printf("First child Contents: %s\n", c0->contents);
            // printf("First child Number of children: %i\n", c0->children_num);
            // printf("children: %s\n", c0->children[0]);

            
            lval result = eval(a);
            lval_println(result);
            /* print and delete AST*/
            // mpc_ast_print(a);
            mpc_ast_delete(a);
        } 
        else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }
    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispc);
    return 0;
}