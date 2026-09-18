#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct job {
    char *nimi;
    char *out;
    size_t len;
};

void *pura(void *arg) {
    struct job *j = arg;
    FILE *fp;
    int count;
    char ch;
    size_t kap = 1024;
    size_t n = 0;
    int k;

    j->out = malloc(kap);
    j->len = 0;

    fp = fopen(j->nimi, "r");
    if (fp == NULL) {
        fprintf(stderr, "punzip: cannot open file\n");
        exit(1);
    }

    while (fread(&count, sizeof(int), 1, fp) == 1) {
        if (fread(&ch, 1, 1, fp) != 1)
            break;
        for (k = 0; k < count; k++) {
            if (n + 1 > kap) {
                kap *= 2;
                j->out = realloc(j->out, kap);
            }
            j->out[n++] = ch;
        }
    }
    fclose(fp);
    j->len = n;
    return NULL;
}

int main(int argc, char *argv[]) {
    int i, n;
    pthread_t *tid;
    struct job *jobs;

    if (argc < 2) {
        fprintf(stderr, "punzip: file1 [file2 ...]\n");
        exit(1);
    }

    n = argc - 1;
    tid = malloc(n * sizeof(pthread_t));
    jobs = malloc(n * sizeof(struct job));

    // jokainen tiedosto omassa säikeessä
    for (i = 0; i < n; i++) {
        jobs[i].nimi = argv[i + 1];
        pthread_create(&tid[i], NULL, pura, &jobs[i]);
    }

    for (i = 0; i < n; i++)
        pthread_join(tid[i], NULL);

    for (i = 0; i < n; i++) {
        fwrite(jobs[i].out, 1, jobs[i].len, stdout);
        free(jobs[i].out);
    }

    free(tid);
    free(jobs);
    return 0;
}
