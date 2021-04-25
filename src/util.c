#include <stdio.h>
#include <stdlib.h>

/**
 * Return the minimum of a and b
 */
int min(int a, int b) {
    return a < b ? a : b;
}

/**
 * Return the maximum of a and b
 */
int max(int a, int b) {
    return a > b ? a : b;
}

/**
 * Print current ERRNO and exit
 * Use this as a last resort when all is lost
 */
void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
