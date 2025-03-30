#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ecl/ecl.h>


#include "alg.h"
#include "test12.h"
#define DATASET_DIR "./dataset_20"
#define DPP 0

FILE *csv;
float current_u;
float current_uc;
char buf_test1[50];
char buf_test2[50];
char buf_sys[2000];


int main(int argc, char *argv[]) {

    DIR *FD = opendir(DATASET_DIR);
    if (!FD) return 1;
    struct dirent* in_file;
    char fname[255];
    csv = fopen("assign.csv", "w");
    fprintf(csv, "n;m;u;uc;assign;test1;test2;alg;group_count;runtime;system;group\n");


    while ((in_file = readdir(FD))) {
        if (!strcmp(in_file->d_name, "."))
            continue;
        if (!strcmp(in_file->d_name, ".."))
            continue;
        if (!strstr(in_file->d_name, ".lsp"))
            continue;

        sprintf(fname, "%s/%s", DATASET_DIR, in_file->d_name);

        printf("Opening %s...\n", fname);
        FILE *f = fopen(fname, "r");
        fseek(f, 0, SEEK_END);
        int sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        char *data = (char*)malloc(sz);
        fread(data, sz, 1, f);
        fclose(f);

        if (prepare(argc, argv)) {
            return 1;
        }


        cl_object my_dataset = c_string_to_object(data);
        cl_object result = cl_eval(my_dataset);

        int count = 0;
        if (ecl_t_of(result) == t_list) {
            do {
                count++;
                cl_object taskset = ecl_car(result);
                if (ecl_t_of(taskset) == t_structure) {
                    ecl_print(taskset, ECL_T);

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

                    System sys = newEmptySystem();
                    sys.n_tasks = n;

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
                            sys.tasks[i].c = c;
                            sys.tasks[i].d = d;
                            sys.tasks[i].u = 1.0 * c / d;
                            sys.tasks[i].p = i;
                            i++;
                        } else break;
                        tasks = ecl_cdr(tasks);
                    } while(true);

                    current_u = u;
                    current_uc = uc;

                    if (n == m - 1)
                        AlgReturn2Str(do_test1(sys), buf_test1);
                    else strcpy(buf_test1, "");


                    AlgReturn2Str(do_test2(sys), buf_test2);
                    printSystemToBuf(sys, buf_sys);

                    Group G = Assignment(sys, m);
                    if (G.n_sys == 0) {
                        printf("\nASSIGNMENT FAILED\n");
                    } else {
                        printf("\nASSIGNMENT OK |G| = %d\n", G.n_sys);
                        printGroup(G);
                    }


                } else break;
                result = ecl_cdr(result);
            } while (true);
        }
        free(data);
    }

}
