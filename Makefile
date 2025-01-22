CFLAGS=-Wall -Wextra -Werror -std=c11 -pedantic -ggdb -Wno-unused-variable 

main: main.c mpc.c mpc.h
	$(CC) $(CFLAGS) -o main main.c mpc.c -ledit -lm 