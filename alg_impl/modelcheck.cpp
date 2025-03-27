#include "modelcheck.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <cerrno>

#define SPIN_CMD "/usr/local/bin/spin -a NP-GFP.pml"
#define SPIN_CMD_COMPILE "gcc -DMEMLIM=8096 -O2 -DXUSAFE -w -o pan pan.c"
#define SHELL "bash"
#define PAN "./pan -m100000000 -a > pan.out"
#define MAX_WAIT_SEC 120


bool compile_spin(System T, int m) {

    FILE * f = fopen("gen.pml", "w");
    fprintf(f, "#define NumProc %d\n", m);
    fprintf(f, "#define NumTask %d\n", T.n_tasks);
    fprintf(f, "inline setup() {\n");
    for (int i = 0; i < T.n_tasks; i++) {
        fprintf(f, " C_i[%d] = %d\n", i, T.tasks[i].c);
        fprintf(f, " D_i[%d] = %d\n", i, T.tasks[i].d);
    }
    fprintf(f, "}\n");
    fclose(f);


    int r = system(SPIN_CMD);
    if (r != 0) {
        printf("error exec spin -a!\n");
        return false;
    }
    r = system(SPIN_CMD_COMPILE);
    if (r != 0) {
        printf("error exec gcc pan!\n");
        return false;
    }
    return true;
}


AlgReturn run_verification(int * runtime) {
    const char* executable = SHELL;
    char* params[] = { (char*)executable, "-c", PAN, NULL };

    int waitSeconds = MAX_WAIT_SEC;
    int realWaitSeconds = waitSeconds;

    pid_t pid = fork();
    if (pid < 0) {
        printf("Fork failed!\n");
        exit(1);
    }

    if (pid == 0) {
        // Child process
        execvp(executable, params);
        printf("exec failed %s!\n", strerror(errno));
        exit(1);
    } else {
        int status;
        char cmd[100];

        printf("Waiting...\n");
        for (int w = 0; w < waitSeconds; w++) {
            sleep(1);
            sprintf(cmd, "ps %d | grep -q 'defunct'", pid);
            int k = system(cmd);
            if (k == 0) { // Check if the child is still running
                realWaitSeconds = w;
                break;
            }
        }
        if (kill(pid, 0) == 0) {
            printf("Terminating the process...\n");
            kill(pid, SIGTERM);
        }

        waitpid(pid, &status, 0);

        printf("SPIN run for %d seconds!\n", realWaitSeconds);

        *runtime = realWaitSeconds;

        FILE *fout = fopen("pan.out", "r");
        fseek(fout, 0, SEEK_END);
        long sz = ftell(fout);
        fseek(fout, 0, SEEK_SET);
        char *buf = (char *)malloc(sz + 1);
        memset(buf, 0, sz);
        fread(buf, sz, 1, fout);
        fclose(fout);
        AlgReturn ret = AlgReturn::unknown;
        if (strstr(buf, "assertion violated") != NULL) {
            ret = AlgReturn::infeasible;
        } else
            if (strstr(buf, "error:") != NULL) {
                ret = AlgReturn::unknown;
            } else {
                ret = AlgReturn::schedulable;
            }
        return ret;
    }
    return AlgReturn::unknown;
}


