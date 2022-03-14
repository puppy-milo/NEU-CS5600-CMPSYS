#include <stdio.h>
#include <stdlib.h>
#include "caesar.h"
#include "string.h"

void usage() {
    printf("Usage: ./caesar [encode|decode] <number> <string> \n");
}

int main(int argc, const char *argv[]) {
    if (argc < 4) {
        usage();
        return -1;
    }

    const char *mode = argv[1];
    int key = atoi(argv[2]);
    char str[128];
    strcpy(str, argv[3]);

    if (!strcmp(mode, "encode")) {
        char *cipher_text = encode(str, key);
        printf("%s\n", cipher_text);
    } else if (!strcmp(mode, "decode")) {
        char *decoded_text = decode(str, key);
        printf("%s\n", decoded_text);
    }
    return 0;
}
