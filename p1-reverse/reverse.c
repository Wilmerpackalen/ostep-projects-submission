#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// linkitetty lista riveille
struct node {
    char *line;
    struct node *next;
};

int main(int argc, char *argv[]) {
    FILE *in = stdin;
    FILE *out = stdout;

    if (argc > 3) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    if (argc >= 2) {
        in = fopen(argv[1], "r");
        if (in == NULL) {
            fprintf(stderr, "error: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
    }

    if (argc == 3) {
        // sama tiedosto? tsekataan inode
        struct stat s1, s2;
        if (stat(argv[1], &s1) == 0 && stat(argv[2], &s2) == 0) {
            if (s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino) {
                fprintf(stderr, "Input and output file must differ\n");
                exit(1);
            }
        }

        out = fopen(argv[2], "w");
        if (out == NULL) {
            fprintf(stderr, "error: cannot open file '%s'\n", argv[2]);
            exit(1);
        }
    }

    struct node *head = NULL;
    char *buf = NULL;
    size_t n = 0;
    ssize_t len;

    // luetaan tiedosto, lisätään aina eteen niin järjestys kääntyy
    while ((len = getline(&buf, &n, in)) != -1) {
        char *copy = malloc(strlen(buf) + 1);
        if (copy == NULL) {
            fprintf(stderr, "malloc failed\n");
            exit(1);
        }
        strcpy(copy, buf);

        struct node *uusi = malloc(sizeof(struct node));
        if (uusi == NULL) {
            fprintf(stderr, "malloc failed\n");
            exit(1);
        }
        uusi->line = copy;
        uusi->next = head;
        head = uusi;
    }

    free(buf);

    struct node *p = head;
    while (p != NULL) {
        fprintf(out, "%s", p->line);
        p = p->next;
    }

    if (argc >= 2)
        fclose(in);
    if (argc == 3)
        fclose(out);

    return 0;
}
