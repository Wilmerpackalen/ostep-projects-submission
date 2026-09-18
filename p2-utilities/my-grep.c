#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char *line = NULL;
    size_t n = 0;
    FILE *fp;
    int i;

    if (argc < 2) {
        fprintf(stderr, "my-grep: searchterm [file ...]\n");
        exit(1);
    }

    if (argc == 2) {
        // ei tiedostoja niin stdin
        while (getline(&line, &n, stdin) != -1) {
            if (strstr(line, argv[1]) != NULL) {
                printf("%s", line);
            }
        }
        free(line);
        return 0;
    }

    for (i = 2; i < argc; i++) {
        fp = fopen(argv[i], "r");
        if (fp == NULL) {
            fprintf(stderr, "my-grep: cannot open file\n");
            exit(1);
        }
        while (getline(&line, &n, fp) != -1) {
            if (strstr(line, argv[1]) != NULL) {
                printf("%s", line);
            }
        }
        fclose(fp);
    }

    free(line);
    return 0;
}
