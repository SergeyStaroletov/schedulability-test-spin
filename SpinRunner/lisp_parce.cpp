#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

#include <ecl/ecl.h>
#define DPP 0
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <cerrno>
#include <cstring>

#define SPIN_CMD "/usr/local/bin/spin -a NP-GFP.pml"
#define SPIN_CMD_COMPILE "gcc -DMEMLIM=8096 -O2 -DXUSAFE -w -o pan pan.c"
#define SHELL "bash"
#define PAN "./pan -m100000000 -a > pan.out"

#define MAX_WAIT_SEC 120
#define DATASET_DIR "/Users/sergey/Projects/ECRTS2025/datasets"

struct Task {
    int c;
    int d;
    int t;
};

enum AlgReturn {
    infeasible,
    schedulable,
    unknown
};


bool compile_spin() {
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


AlgReturn run_verification(int n, int m, struct Task *Tasks) {

    const char* executable = SHELL;
    //./pan -m100000000  -a
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
        char buf[1024];

        printf("Waiting...\n");
        for (int w = 0; w < waitSeconds; w++) {
            sleep(1);
            sprintf(buf, "ps %d | grep -q 'defunct'", pid);
            int k = system(buf);
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

        FILE *fout = fopen("pan.out", "r");
        memset(buf, 0, sizeof(buf));
        fread(buf, sizeof(buf), 1, fout);
        fclose(fout);
        FILE *frez;
        AlgReturn ret = AlgReturn::unknown;
        if (strstr(buf, "assertion violated") != NULL) {
            frez = fopen("infeasible.txt", "a");
            ret = AlgReturn::infeasible;
        } else
            if (strstr(buf, "pan: elapsed time") != NULL) {
                frez = fopen("ok.txt", "a");
                ret = AlgReturn::schedulable;
            } else {
                frez = fopen("unknown.txt", "a");
                ret = AlgReturn::unknown;
            }
        fprintf(frez, "M:%d N:%d ", m, n);
        for (int i = 0; i < n; i++)
            fprintf(frez, "(C:%d D:%d) ", Tasks[i].c, Tasks[i].d);
        fprintf(frez, "\n");
        fclose(frez);
        return ret;
    }
    return AlgReturn::unknown;
}


int main(int argc, char *argv[])
{
    system("rm infeasible.txt");
    system("rm unknown.txt");
    system("rm ok.txt");


    DIR *FD = opendir(DATASET_DIR);
    if (!FD) return 1;
    struct dirent* in_file;
    char fname[255];

    while ((in_file = readdir(FD))) {

        if (!strcmp(in_file->d_name, "."))
            continue;
        if (!strcmp(in_file->d_name, ".."))
            continue;
        if (!strstr(in_file->d_name, ".lsp"))
            continue;


        int b = cl_boot(argc, argv);
        if (!b) {
            printf("ECL is not loaded!\n");
            return 1;
        }
        sprintf(fname, "%s/%s", DATASET_DIR, in_file->d_name);

        printf("Opening %s...\n", fname);

        FILE *f = fopen(fname, "r");
        fseek(f, 0, SEEK_END);
        int sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        char *data = (char*)malloc(sz);
        fread(data, sz, 1, f);
        fclose(f);

        cl_object result = cl_eval(c_string_to_object("(defstruct (task) C D T)"));
        result = cl_eval(c_string_to_object("(defstruct (taskset) M N U UC tasks)"));

        cl_object my_dataset = c_string_to_object(data);
        result = cl_eval(my_dataset);

        cl_eval(c_string_to_object("(defun implies (x y)"
                                    "(cond ((equal x nil) T)"
                                   "(T y)))"));

        cl_eval(c_string_to_object(
            "(defun Alg1 (ts)"
            "  (do ((n (taskset-N ts))"
            "       (tasks (taskset-tasks ts))"
            "       (i 1 (+ i 1)))"
            "      ((> i n))"
            "    (do ((j 1 (+ j 1)))"
            "	((> j n) (return-from Alg1 'infeasible))"
            "      (let ((Ci (task-C (nth (- i 1) tasks)))"
            "	    (Cj (task-C (nth (- j 1) tasks)))"
            "	    (Di (task-D (nth (- i 1) tasks))))"
            "	(if (not (implies"
            "		  (not (equal i j))"
            "		  (or (< Di Cj) (< Di (+ Cj Ci)))))"
            "	    (return))))"
            "    (do ((j 1 (+ j 1)))"
            "	((>= j i))"
            "      (let ((Ci (task-C (nth (- i 1) tasks)))"
            "	    (Cj (task-C (nth (- j 1) tasks)))"
            "	    (Di (task-D (nth (- i 1) tasks)))"
            "	    (Dj (task-D (nth (- j 1) tasks))))"
            "	(if (and"
            "	     (or"
            "	      (and (<= Cj Di) (< Di (* 2 Cj)))"
            "	      (and (<= (+ Cj Ci) Di) (< Di (+ (* 2 Cj) Ci))))"
            "	     (> Ci (- Dj Cj)))"
            "	    (return-from Alg1 'infeasible)))))"
            "  (return-from Alg1 'schedulable))"
        ));


        cl_eval(c_string_to_object(
            "(defun Alg2 (ts)"
            "  (do ((n (taskset-N ts))"
            "       (tasks (taskset-tasks ts))"
            "       (i 1 (+ i 1)))"
            "      ((> i n))"
            "    (do ((j 1 (+ j 1)))"
            "	((> j n) (return-from Alg2 'infeasible))"
            "      (let ((Cj (task-C (nth (- j 1) tasks)))"
            "	    (Di (task-D (nth (- i 1) tasks)))"
            "       (Ci (task-C (nth (- i 1) tasks))))"
            "	(if (not (implies"
            "		  (not (equal i j))"
            "		  (or (< Di Cj) (< Di (+ Cj Ci)))))"
            "	    (return))))"
            "    (do ((j 1 (+ j 1)))"
            "	((>= j i))"
            "      (let ((Ci (task-C (nth (- i 1) tasks)))"
            "	    (Cj (task-C (nth (- j 1) tasks)))"
            "	    (Di (task-D (nth (- i 1) tasks)))"
            "	    (Dj (task-D (nth (- j 1) tasks))))"
            "	(if (and"
            "	     (or"
            "	      (and (<= Cj Di) (< Di (* 2 Cj)))"
            "	      (and (<= (+ Cj Ci) Di) (< Di (+ (* 2 Cj) Ci))))"
            "	     (> Ci (- Dj Cj)))"
            "	    (return-from Alg2 'infeasible)))))"
            "  (return-from Alg2 'unknown))"
        ));


        int count = 0;
        if (ecl_t_of(result) == t_list) {
            do {
                count++;
                cl_object taskset = ecl_car(result);
                if (ecl_t_of(taskset) == t_structure) {
                   //ecl_print(taskset, ECL_T);
                   int m = ecl_fixnum(taskset->instance.slots[0]);
                   int n = ecl_fixnum(taskset->instance.slots[1]);
                   float u = ecl_single_float(taskset->instance.slots[2]);
                   if (u > 1) {
                       printf("Wrong utilization u = %f!\n", u);
                       break;
                   }
                   float uc = ecl_single_float(taskset->instance.slots[3]);
                   printf("\n[%d] Parsed taskset: M=%d N=%d U=%f UC=%f\n", count, m, n, u, uc);
                   cl_object tasks = taskset->instance.slots[4];

                   struct Task *Tasks = (struct Task *)malloc(n * sizeof(struct Task));
                   int i = 0;
                   do {
                       cl_object task = ecl_car(tasks);
                       if (ecl_t_of(task) == t_structure) {
                           //ecl_print(task, ECL_T);
                           int c = ecl_fixnum(task->instance.slots[0]);
                           int d = ecl_fixnum(task->instance.slots[1]);
                           int t = ecl_fixnum(task->instance.slots[2]);
                           //printf("\n Parsed task: C=%d D=%d T=%d\n", c, d, t);
                           if (c == d) {
                               d++;
                               t++;
                           }
                           if (i == n) {
                               printf("wrong number of tasks!\n");
                               break;
                           }
                           Tasks[i].c = c;
                           Tasks[i].d = d;
                           Tasks[i].t = t;
                           i++;
                       } else break;
                       tasks = ecl_cdr(tasks);
                   } while(true);
                   f = fopen("gen.pml", "w");
                   fprintf(f, "#define NumProc %d\n", m);
                   fprintf(f, "#define NumTask %d\n", n);
                   fprintf(f, "inline setup() {\n");
                   for (int i = 0; i < n; i++) {
                       fprintf(f, " C_i[%d] = %d\n", i, Tasks[i].c);
                       fprintf(f, " D_i[%d] = %d\n", i, Tasks[i].d);
                   }
                   fprintf(f, "}\n");
                   fclose(f);

                   cl_object rez_alg1 = cl_funcall(2, c_string_to_object("Alg1"), taskset);
                   cl_object rez_alg2 = cl_funcall(2, c_string_to_object("Alg2"), taskset);

                   bool infeasible_alg2 = !strcmp((const char *)rez_alg2->symbol.name->string.self, "INFEASIBLE");
                   AlgReturn alg2_ret = infeasible_alg2 ? AlgReturn::infeasible : AlgReturn::unknown;
                   bool shed_alg1 = !strcmp((const char *)rez_alg1->symbol.name->string.self, "SCHEDULABLE");
                   {
                       if (compile_spin()) {
                           char spin_ret_str[50];
                           AlgReturn spin_ret = run_verification(n, m, Tasks);
                           if (spin_ret == AlgReturn::infeasible)
                               strcpy(spin_ret_str, "infeasible");
                           if (spin_ret == AlgReturn::unknown)
                               strcpy(spin_ret_str, "unknown");
                           if (spin_ret == AlgReturn::schedulable)
                               strcpy(spin_ret_str, "schedulable");
                           char genstr1[50];
                           char genstr2[50];
                           char genstr3[50];

                           sprintf(genstr1, "cp gen.pml gen_%lu.pml", time(NULL));
                           sprintf(genstr2, "cp pan.out pan_%lu.out", time(NULL));
                           sprintf(genstr3, "cp NP-GFP.pml.trail NP-GFP_%lu.trail", time(NULL));


                           if (m == n - 1) {
                               if ((spin_ret == AlgReturn::schedulable && alg2_ret == AlgReturn::infeasible) ||
                                   (spin_ret == AlgReturn::infeasible && alg2_ret == AlgReturn::unknown)
                                   ) {
                                   printf("\n\n***!!!!!!Wrong SPIN result for m=n-1: SPIN=%s/ALG2=%s\n***", spin_ret_str,
                                          infeasible_alg2? "infeasible": "unknown");
                                   ecl_print(taskset, ECL_T);
                                   system(genstr1);
                                   system(genstr2);
                                   system(genstr3);
                               }
                           } else {
                               if (spin_ret == AlgReturn::schedulable && alg2_ret == AlgReturn::infeasible) {
                                   printf("\n\n***!!!!!!Wrong SPIN result for m<n: SPIN=%s/ALG2=%s\n***", spin_ret_str,
                                          infeasible_alg2? "infeasible": "unknown");
                                   ecl_print(taskset, ECL_T);
                                   system(genstr1);
                                   system(genstr2);
                                   system(genstr3);

                               }
                           }

                           if (spin_ret == AlgReturn::schedulable && (!shed_alg1)) {
                               printf("\n\n***!!Wrong SPIN result Alg1: not SCHEDULABLE!!\n");
                               ecl_print(taskset, ECL_T);
                               system(genstr1);
                               system(genstr2);
                               system(genstr3);
                           }
                   }

                   }
                   free(Tasks);

                   //ecl_print(rez_alg2, ECL_T);


                } else break;
                result = ecl_cdr(result);
            } while (true);
        }
        free(data);
        cl_shutdown();
    }

}
