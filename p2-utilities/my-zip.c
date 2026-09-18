#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *fp;
    int i, c;
    int count = 0;
    int prev = -1;

    if (argc < 2) {
        fprintf(stderr, "my-zip: file1 [file2 ...]\n");
        exit(1);
    }

    for (i = 1; i < argc; i++) {
        fp = fopen(argv[i], "r");
        if (fp == NULL) {
            fprintf(stderr, "my-zip: cannot open file\n");
            exit(1);
        }

        while ((c = fgetc(fp)) != EOF) {
            if (prev == -1) {
                prev = c;
                count = 1;
            } else if (c == prev) {
                count++;
            } else {
                // 4 tavua count + 1 tavu merkki
                fwrite(&count, sizeof(int), 1, stdout);
                fwrite(&prev, 1, 1, stdout);
                prev = c;
                count = 1;
            }
        }
        fclose(fp);
    }

    if (prev != -1) {
        fwrite(&count, sizeof(int), 1, stdout);
        fwrite(&prev, 1, 1, stdout);
    }

    return 0;
}
