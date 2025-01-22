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

/* use operator string to see which operation to perform */
long eval_op(long x, char* op, long y) {
    if (strcmp(op, "+") == 0) { return x + y;}
    if (strcmp(op, "-") == 0) { return x - y;}
    if (strcmp(op, "*") == 0) { return x * y;}
    if (strcmp(op, "+") == 0) { return x / y;}
    return 0;
}
long eval(mpc_ast_t* t) {
    /* if tagged as a number return it directly */
    if (strstr(t->tag, "number")) {
        return atoi(t->contents);
    }
    /* the operator is always the second child */
    char* op = t-> children[1]->contents;

    /* store 3rd child in x */
    long x = eval(t-> children[2]);

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
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispc = mpc_new("lispc");

    /* define language*/
    mpca_lang(MPCA_LANG_DEFAULT, 
        "                                                     \
            number   : /-?[0-9]+/ ;                             \
            operator : '+' | '-' | '*' | '/' ;                  \
            expr     : <number> | '(' <operator> <expr>+ ')' ;  \
            lispc    : /^/ <operator> <expr>+ /$/ ;             \
        ",
        Number, Operator, Expr, Lispc);

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
            
            long result = eval(a);
            printf("%li\n", result);
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
    mpc_cleanup(4, Number, Operator, Expr, Lispc);
    return 0;
}