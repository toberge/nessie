#include "nessie.h"
#include <stdio.h>
#include <unistd.h>

int cd(char **argv, int argc) {
    if (argc == 2) {
        return -chdir(argv[1]);
    } else {
        printf("cd: Invalid number of arguments");
        return 1;
    }
}
