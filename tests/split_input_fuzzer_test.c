#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

char **split_input(const char *input, const int len, int *num_tokens);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // constrain to null-terminated strings
    char *instring = malloc(sizeof(char) * (size + 1));
    memcpy(instring, data, size); // copy memory chunk
    instring[size] = '\0'; // null-terminate
    
    int num_tokens;
    char **tokens = split_input(instring, size, &num_tokens);
    free(tokens);
    free(instring);
    return 0;
}
