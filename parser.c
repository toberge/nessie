#include "nessie.h"
#include <string.h>
#include <strings.h>

// Stupidly simple look-for-pipe thing right now
int parse_and_execute(char **argv, int argc) {
    for (int i = 1; i < argc-1; i++) {
        if (strcmp(argv[i], "|") == 0) {
            int jmp = argc-i-1;
            return pipe_commands(argv, i, argv+jmp, argc+jmp);
        }
    }
    return launch_command(argv, argc);
}
