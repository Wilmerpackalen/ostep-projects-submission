#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>

char error_message[30] = "An error has occurred\n";

void print_error() {
    write(STDERR_FILENO, error_message, strlen(error_message));
}

char **paths = NULL;
int npaths = 0;

void set_path(char **p, int n) {
    int i;
    for (i = 0; i < npaths; i++)
        free(paths[i]);
    free(paths);
    paths = NULL;
    npaths = 0;
    if (n == 0)
        return;
    paths = malloc(n * sizeof(char *));
    for (i = 0; i < n; i++)
        paths[i] = strdup(p[i]);
    npaths = n;
}

char *find_cmd(char *cmd) {
    int i;
    char buf[1024];
    for (i = 0; i < npaths; i++) {
        sprintf(buf, "%s/%s", paths[i], cmd);
        if (access(buf, X_OK) == 0)
            return strdup(buf);
    }
    return NULL;
}

int empty_line(char *s) {
    int i;
    for (i = 0; s[i]; i++) {
        if (!isspace((unsigned char)s[i]))
            return 0;
    }
    return 1;
}

void run_line(char *line) {
    char *tok[200];
    int ntok = 0;
    char *save;
    char *t;
    int i, k;
    pid_t pids[100];
    int npids = 0;

    if (empty_line(line))
        return;

    // pistetään välilyönnit & ja > ympärille
    char tmp[4096];
    int j = 0;
    for (i = 0; line[i] != '\0' && j < 4090; i++) {
        if (line[i] == '&' || line[i] == '>') {
            tmp[j++] = ' ';
            tmp[j++] = line[i];
            tmp[j++] = ' ';
        } else {
            tmp[j++] = line[i];
        }
    }
    tmp[j] = '\0';

    t = strtok_r(tmp, " \t\n", &save);
    while (t != NULL) {
        tok[ntok++] = t;
        t = strtok_r(NULL, " \t\n", &save);
    }
    if (ntok == 0)
        return;

    int start = 0;
    int cmd_a[100], cmd_b[100], ncmd = 0;
    for (i = 0; i <= ntok; i++) {
        if (i == ntok || strcmp(tok[i], "&") == 0) {
            if (i > start) {
                cmd_a[ncmd] = start;
                cmd_b[ncmd] = i;
                ncmd++;
            } else if (i != ntok) {
                print_error();
                return;
            }
            start = i + 1;
        }
    }

    for (k = 0; k < ncmd; k++) {
        char *args[200];
        int nargs = 0;
        char *redir = NULL;
        int err = 0;

        for (i = cmd_a[k]; i < cmd_b[k]; i++) {
            if (strcmp(tok[i], ">") == 0) {
                if (redir != NULL || i + 1 >= cmd_b[k] || i + 2 != cmd_b[k]) {
                    print_error();
                    err = 1;
                    break;
                }
                redir = tok[i + 1];
                i++;
            } else {
                args[nargs++] = tok[i];
            }
        }
        if (err)
            break;
        if (nargs == 0) {
            print_error();
            break;
        }
        args[nargs] = NULL;

        if (strcmp(args[0], "exit") == 0) {
            if (nargs != 1 || redir != NULL) {
                print_error();
                continue;
            }
            exit(0);
        }
        if (strcmp(args[0], "cd") == 0) {
            if (nargs != 2 || redir != NULL) {
                print_error();
                continue;
            }
            if (chdir(args[1]) != 0)
                print_error();
            continue;
        }
        if (strcmp(args[0], "path") == 0) {
            if (redir != NULL) {
                print_error();
                continue;
            }
            set_path(args + 1, nargs - 1);
            continue;
        }

        char *bin = find_cmd(args[0]);
        if (bin == NULL) {
            print_error();
            continue;
        }

        // luodaan lapsiprosessi
        pid_t pid = fork();
        if (pid == 0) {
            if (redir != NULL) {
                int fd = open(redir, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (fd < 0) {
                    print_error();
                    _exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);
                close(fd);
            }
            execv(bin, args);
            print_error();
            _exit(1);
        }
        if (pid < 0)
            print_error();
        else
            pids[npids++] = pid;
        free(bin);
    }

    // odotetaan lapset
    for (i = 0; i < npids; i++)
        waitpid(pids[i], NULL, 0);
}

int main(int argc, char *argv[]) {
    char *def[] = {"/bin"};
    set_path(def, 1);

    FILE *in = stdin;
    int interactive = 1;
    char *line = NULL;
    size_t n = 0;

    if (argc == 2) {
        in = fopen(argv[1], "r");
        if (in == NULL) {
            print_error();
            exit(1);
        }
        interactive = 0;
    } else if (argc > 2) {
        print_error();
        exit(1);
    }

    while (1) {
        if (interactive) {
            printf("wish> ");
            fflush(stdout);
        }
        if (getline(&line, &n, in) == -1)
            break;
        run_line(line);
    }

    exit(0);
}
