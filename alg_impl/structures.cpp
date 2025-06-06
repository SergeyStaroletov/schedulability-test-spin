#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "structures.h"


System newEmptySystem() {
    System newSystem;
    newSystem.n_tasks = 0;
    return newSystem;
}

bool empty(System T) {
    return T.n_tasks == 0;
}

Task head(System T) {
    return T.tasks[0];
}


int cmpByUUp(const void *a, const void *b) {
    Task *task1 = (Task *)a;
    Task *task2 = (Task *)b;
    if (task2->u - task1->u > 0) return 1;
    if (task2->u - task1->u < 0) return -1;
    return 0;
}

int cmpByUDown(const void *a, const void *b) {
    Task *task1 = (Task *)a;
    Task *task2 = (Task *)b;
    if (task1->u - task2->u > 0) return 1;
    if (task1->u - task2->u < 0) return -1;
    return 0;
}

int cmpByDUp(const void *a, const void *b) {
    Task *task1 = (Task *)a;
    Task *task2 = (Task *)b;
    return task2->d - task1->d;
}

int cmpByDDown(const void *a, const void *b) {
    Task *task1 = (Task *)a;
    Task *task2 = (Task *)b;
    return task1->d - task2->d;
}

int cmpByP(const void *a, const void *b) {
    Task *task1 = (Task *)a;
    Task *task2 = (Task *)b;
    return task1->p - task2->p;
}

System sort(System T, Sorting byWhat, UpDown UpDn) {

    if (byWhat == Sorting::byU && UpDn == UpDown::Up)
        qsort(&T.tasks, T.n_tasks, sizeof(Task), &cmpByUUp);
    else if (byWhat == Sorting::byU && UpDn == UpDown::Down)
        qsort(&T.tasks, T.n_tasks, sizeof(Task), &cmpByUDown);
    else if (byWhat == Sorting::byD && UpDn == UpDown::Up)
        qsort(&T.tasks, T.n_tasks, sizeof(Task), &cmpByDUp);
    else if (byWhat == Sorting::byD && UpDn == UpDown::Down)
        qsort(&T.tasks, T.n_tasks, sizeof(Task), &cmpByDDown);
    else if (byWhat == Sorting::byP)
        qsort(&T.tasks, T.n_tasks, sizeof(Task), &cmpByP);

    return T;
}

int pwr(System T) {
    return T.n_tasks;
}

bool theSameTask(Task t1, Task t2) {
    return (t1.c == t2.c) && (t1.d == t2.d) && (t1.p == t2.p);
}

System first(System T, int n) {
    System newSystem = newEmptySystem();
    if (n > T.n_tasks) n = T.n_tasks; //?
    newSystem.n_tasks = n;
    for (int i = 0; i < n; i++) {
        newSystem.tasks[i] = T.tasks[i];
    }
    return newSystem;
}

System removeTasks(System T, System T1) {
    int count = 0;
    System newSystem = newEmptySystem();
    for (int i = 0; i < T.n_tasks; i++) {
        bool found = false;
        for (int j = 0; j < T1.n_tasks; j++)
            if (theSameTask(T.tasks[i], T1.tasks[j])) {
                found = true;
                break;
            }
        if (!found) newSystem.tasks[count++] = T.tasks[i];
    }
    newSystem.n_tasks = count;
    return newSystem;
}

System removeTask(System T, Task t) {
    System T2 = newEmptySystem();
    T2.tasks[0] = t;
    T2.n_tasks = 1;
    return removeTasks(T, T2);
}

System addTasks(System T, System T1) {
    System newSystem = newEmptySystem();
    int count = 0;
    //добавить все из T
    for (int i = 0; i < T.n_tasks; i++)
        newSystem.tasks[count++] = T.tasks[i];
    //добавить все из T2, которых нет в T1
    for (int i = 0; i < T1.n_tasks; i++) {
        bool found = false;
        for (int j = 0; j < T.n_tasks; j++)
            if (theSameTask(T1.tasks[i], T.tasks[j])) {
                found = true;
                break;
            }
        if (!found) newSystem.tasks[count++] = T1.tasks[i];
    }
    newSystem.n_tasks = count;
    return newSystem;
}

System addTask(System T, Task t) {
    System T2 = newEmptySystem();
    T2.tasks[0] = t;
    T2.n_tasks = 1;
    return addTasks(T, T2);
}

System removeFirst(System T, int n) {
    System newSystem = newEmptySystem();
    int count = 0;
    for (int i = 0; i < T.n_tasks - n; i++)
        if (i + n <= T.n_tasks) {
            newSystem.tasks[i] = T.tasks[i + n];
            count++;
        }
    newSystem.n_tasks = count;
    return newSystem;
}

System removeLast(System T) {
    System newSystem = T;
    if (newSystem.n_tasks != 0) newSystem.n_tasks--;
    return newSystem;
}

System replace(System T, int k, Task t) {
    System newSystem = T;
    newSystem.tasks[k] = t;
    return newSystem;
}

float USystem(System T) {
    double u = 0;
    for (int j = 0; j < T.n_tasks; j++)
        u += T.tasks[j].u;
    return u;
}

void printSystem(const char * label, System T) {
    printf("%s = {pwr: %d ", label, T.n_tasks);
    int n = T.n_tasks;
    for (int i = 0; i < n; i++) {
        printf("  (n:%d c:%d d:%d u:%f)", T.tasks[i].p, T.tasks[i].c, T.tasks[i].d, T.tasks[i].u);
        if (i < n - 1) printf(", \n");
    }
    printf("}\n");
}

Group newEmptyGroup() {
    Group newG;
    newG.n_sys = 0;
    return newG;
}

Group addSystemToGroup(System T, Group g, int cpus) {
    g.processors[g.n_sys] = cpus;
    g.systems[g.n_sys++] = T;
    return g;
}

Group addGroupToGroup(Group g1, Group g2) {
    Group newG = g1;
    for (int i = 0; i < g2.n_sys; i++) {
        newG.processors[newG.n_sys] = g2.processors[i];
        newG.systems[newG.n_sys++] = g2.systems[i];
    }
    return newG;
}

void printGroup(Group g) {
    printf("Set of %d systems = {", g.n_sys);
    for (int i = 0; i < g.n_sys; i++) {
        char buf[100];
        sprintf(buf, "    System #%d [CPUs = %d]", i, g.processors[i]);
        printSystem(buf, g.systems[i]);
    }
    printf("}\n");
}


void printSystemToBuf(System T, char *buf) {
    std::string result;
    sprintf(buf, "["); //, m, T.n_tasks);//, 0, 0);
    result = std::string(buf);

    for (int i = 0; i < T.n_tasks; i++) {
        sprintf(buf, "(task :N %d :C %d :D %d :U %f) ", T.tasks[i].p, T.tasks[i].c, T.tasks[i].d, T.tasks[i].u);
        result += std::string(buf);
    }

    result += std::string("]");
    strcpy(buf, result.c_str());
}

void printGroupToBuf(Group G, char *buf) {
    std::string result;

    result += std::string("{");

    for (int i = 0; i < G.n_sys; i++) {

        double u = 0;
        for (int j = 0; j < G.systems[i].n_tasks; j++)
            u += G.systems[i].tasks[j].u;
        u /= G.processors[i];

        result += std::string("[CPUs = ") + std::to_string(G.processors[i]) + ", u = " + std::to_string(u) + std::string("],");
        printSystemToBuf(G.systems[i], buf);
        result += std::string(buf) + std::string(" ");
    }

    result += std::string("}");

    strcpy(buf, result.c_str());
}


int printGroupToBufLess(Group G, char *buf) {
    std::string result;
    int cpus = 0;

    result += std::string("{");
    for (int i = 0; i < G.n_sys; i++) {
        result += std::string("[CPUs=") + std::to_string(G.processors[i]) + std::string(",");
        cpus += G.processors[i];
        double u = 0;
        for (int j = 0; j < G.systems[i].n_tasks; j++)
            u += G.systems[i].tasks[j].u;
        u /= G.processors[i];
        result += std::string("NTask=") + std::to_string(G.systems[i].n_tasks) +
                std::string(",U=")+ std::to_string(u) + std::string("]");
    }
    result += std::string("}");

    strcpy(buf, result.c_str());
    return cpus;
}

