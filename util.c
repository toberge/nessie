#include <stdio.h>
#include <stdlib.h>

/**
 * Print current ERRNO and exit
 * Use this as a last resort when all is lost
 */
void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
