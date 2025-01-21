#include <stdio.h>

#define MAXIMUM_SIZE 2048

static char input[MAXIMUM_SIZE]; /* bufer to hold user input of size 2048 */

int main(int arg, char**argv)
{
    puts("lispc version 0.0.0.0.1"); /* version info */
    puts("Press ctrl+c to exit\n");

    while(1) 
    {
        fputs("lispc> ", stdout); /*prompt output */
        fgets(input, MAXIMUM_SIZE, stdin); /* read input */

        printf("No you're a %s", input);
    }
    return 0;
}