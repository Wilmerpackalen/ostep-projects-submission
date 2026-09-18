#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int i, c;
    FILE *fp;

    // ei args -> exit 0
    if (argc == 1) {
        return 0;
    }

    for (i = 1; i < argc; i++) {
        fp = fopen(argv[i], "r");
        if (fp == NULL) {
            fprintf(stderr, "my-cat: cannot open file\n");
            exit(1);
        }
        // tässä luetaan tiedosto
        while ((c = fgetc(fp)) != EOF) {
            putchar(c);
        }
        fclose(fp);
    }

    return 0;
}
