#ifndef STRUCTURES_H
#define STRUCTURES_H

#define MAX_TASK 40
#define MAX_SYS 40

#define MC_MAX 5
// Число задач, для которых можно _доказать_ планируемость формальными методами.
// Сейчас N=5.


struct Task {
    int c;
    int d;
    float u;
    int p;
};

struct System {
    int id;
    int n_tasks;
    struct Task tasks[MAX_TASK];
};


struct Group {
    int n_sys;
    struct System systems[MAX_SYS];
    int processors[MAX_SYS];
};

enum AlgReturn {
    infeasible,
    schedulable,
    unknown
};

enum Sorting {
    byU,
    byD,
    byP
};

enum Alg {
    Alg_MaxBin_T,
    Alg_MidBin_T,//
    Alg_MidBin_ET,//
    Alg_MinBin_ET,
    Alg_MinBin_ET_small
};


System newEmptySystem();
bool empty(System T);
Task head(System T);
int pwr(System T);
System sort(System T, Sorting byWhat, bool UpDown);
int pwr(System T);
bool theSameTask(Task t1, Task t2);
System first(System T, int n);
System removeTasks(System T, System T1);
System removeTask(System T, Task t);
System addTasks(System T, System T1);
System addTask(System T, Task t);
System removeFirst(System T, int n);
System removeLast(System T);
System replace(System T, int k, Task t);
void printSystem(const char * label, System T);
Group newEmptyGroup();
Group addSystemToGroup(System T, Group g, int cpus);
Group addGroupToGroup(Group g1, Group g2);
void printGroup(Group g);

void printSystemToBuf(System T, char *buf);
void printGroupToBuf(Group G, char *buf);
int printGroupToBufLess(Group G, char *buf);

#endif // STRUCTURES_H
