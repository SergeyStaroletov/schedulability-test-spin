#ifndef STRUCTURES_H
#define STRUCTURES_H

#define MAX_TASK 20
#define MAX_SYS 20

#define MC_MAX 6
// Число задач, для которых можно _доказать_ планируемость формальными методами.
// Сейчас N=6.


struct Task {
    int c;
    int d;
    float u;
};

struct System {
    int n_tasks;
    struct Task tasks[MAX_TASK];
};


struct Group {
    int n_sys;
    struct System systems[MAX_SYS];
};

enum AlgReturn {
    infeasible,
    schedulable,
    unknown
};


System newEmptySystem();
bool empty(System T);
Task head(System T);
System sort(System T);
int pwr(System T);
bool theSameTask(Task t1, Task t2);
System first(System T, int n);
System removeTasks(System T, System T1);
System removeTask(System T, Task t);
System addTasks(System T, System T1);
System addTask(System T, Task t);
System removeFirst(System T, int n);
System replace(System T, int k, Task t);
void printSystem(System T);
Group newEmptyGroup();
Group addSystemToGroup(System T, Group g);
Group addGroupToGroup(Group g1, Group g2);
void printGroup(Group g);



#endif // STRUCTURES_H
