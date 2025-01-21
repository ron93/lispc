#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <editline/history.h>


#define MAXIMUM_SIZE 2048

static char input[MAXIMUM_SIZE]; /* bufer to hold user input of size 2048 */

int main(int arg, char**argv)
{
    puts("lispc version 0.0.0.0.1"); /* version info */
    puts("Press ctrl+c to exit\n");

    while(1) 
    {
        char* input = readline("lispc> ");
        add_history(input); /*add input to history-> accessed by pressing the arrow key*/

        printf("No you're a %s\n", input);

        free(input);
    }
    return 0;
}