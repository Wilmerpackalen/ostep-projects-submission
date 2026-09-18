#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *fp;
    int i, j;
    int count;
    char ch;

    if (argc < 2) {
        fprintf(stderr, "my-unzip: file1 [file2 ...]\n");
        exit(1);
    }

    for (i = 1; i < argc; i++) {
        fp = fopen(argv[i], "r");
        if (fp == NULL) {
            fprintf(stderr, "my-unzip: cannot open file\n");
            exit(1);
        }

        while (fread(&count, sizeof(int), 1, fp) == 1) {
            if (fread(&ch, 1, 1, fp) != 1)
                break;
            for (j = 0; j < count; j++)
                putchar(ch);
        }
        fclose(fp);
    }

    return 0;
}
