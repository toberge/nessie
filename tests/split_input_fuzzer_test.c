#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

char **split_input(const char *input, int *num_tokens);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // constrain to strings w/o null bytes inside
    for (int i = 0; i < size; i++)
        if (data[i] == '\0')
            return 0; // gotcha!

    // make all inputs null-terminated strings
    char *instring = malloc(sizeof(char) * (size + 1));
    memcpy(instring, data, size); // copy memory chunk
    instring[size] = '\0'; // null-terminate

    int num_tokens;
    char **tokens = split_input(instring, &num_tokens);
    for (int i = 0; i < num_tokens; i++)
        free(tokens[i]);
    free(tokens);
    free(instring);
    return 0;
}
