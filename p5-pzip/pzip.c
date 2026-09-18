#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sysinfo.h>

struct run {
    int n;
    unsigned char ch;
};

struct job {
    unsigned char *p;
    size_t len;
    struct run *runs;
    int nruns;
};

void *zippaa(void *arg) {
    struct job *j = arg;
    size_t i;
    int kap = 32;
    unsigned char prev;
    int cnt;

    j->runs = malloc(kap * sizeof(struct run));
    j->nruns = 0;
    if (j->len == 0)
        return NULL;

    prev = j->p[0];
    cnt = 1;
    for (i = 1; i < j->len; i++) {
        if (j->p[i] == prev) {
            cnt++;
        } else {
            if (j->nruns >= kap) {
                kap *= 2;
                j->runs = realloc(j->runs, kap * sizeof(struct run));
            }
            j->runs[j->nruns].n = cnt;
            j->runs[j->nruns].ch = prev;
            j->nruns++;
            prev = j->p[i];
            cnt = 1;
        }
    }
    if (j->nruns >= kap) {
        kap *= 2;
        j->runs = realloc(j->runs, kap * sizeof(struct run));
    }
    j->runs[j->nruns].n = cnt;
    j->runs[j->nruns].ch = prev;
    j->nruns++;
    return NULL;
}

int main(int argc, char *argv[]) {
    int i, nfiles, nthreads;
    unsigned char *buf;
    size_t total = 0, pos = 0;
    pthread_t *tid;
    struct job *jobs;

    if (argc < 2) {
        fprintf(stderr, "pzip: file1 [file2 ...]\n");
        exit(1);
    }

    nfiles = argc - 1;

    // ensin koko, sit mmap + kopio yhteen puskuriin
    for (i = 0; i < nfiles; i++) {
        struct stat st;
        if (stat(argv[i + 1], &st) < 0) {
            fprintf(stderr, "pzip: cannot open file\n");
            exit(1);
        }
        total += st.st_size;
    }

    buf = malloc(total ? total : 1);
    for (i = 0; i < nfiles; i++) {
        int fd = open(argv[i + 1], O_RDONLY);
        struct stat st;
        unsigned char *m;
        if (fd < 0 || fstat(fd, &st) < 0) {
            fprintf(stderr, "pzip: cannot open file\n");
            exit(1);
        }
        if (st.st_size > 0) {
            m = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
            if (m == MAP_FAILED) {
                fprintf(stderr, "pzip: cannot open file\n");
                exit(1);
            }
            memcpy(buf + pos, m, st.st_size);
            munmap(m, st.st_size);
            pos += st.st_size;
        }
        close(fd);
    }

    if (total == 0)
        return 0;

    nthreads = get_nprocs();
    if (nthreads < 1)
        nthreads = 1;
    if ((size_t)nthreads > total)
        nthreads = (int)total;

    jobs = malloc(nthreads * sizeof(struct job));
    tid = malloc(nthreads * sizeof(pthread_t));

    {
        size_t chunk = total / nthreads;
        for (i = 0; i < nthreads; i++) {
            jobs[i].p = buf + i * chunk;
            if (i == nthreads - 1)
                jobs[i].len = total - i * chunk;
            else
                jobs[i].len = chunk;
            pthread_create(&tid[i], NULL, zippaa, &jobs[i]);
        }
    }

    for (i = 0; i < nthreads; i++)
        pthread_join(tid[i], NULL);

    // lukitus ei tarvita enää, säikeet on jo valmiita
    for (i = 0; i < nthreads; i++) {
        int k;
        if (i + 1 < nthreads && jobs[i].nruns && jobs[i + 1].nruns) {
            struct run *a = &jobs[i].runs[jobs[i].nruns - 1];
            struct run *b = &jobs[i + 1].runs[0];
            if (a->ch == b->ch) {
                a->n += b->n;
                b->n = 0;
            }
        }
        for (k = 0; k < jobs[i].nruns; k++) {
            if (jobs[i].runs[k].n > 0) {
                fwrite(&jobs[i].runs[k].n, sizeof(int), 1, stdout);
                fwrite(&jobs[i].runs[k].ch, 1, 1, stdout);
            }
        }
        free(jobs[i].runs);
    }

    free(buf);
    free(jobs);
    free(tid);
    return 0;
}
