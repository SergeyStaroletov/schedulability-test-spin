#ifndef STRUCTURES_H
#define STRUCTURES_H

#define MAX_TASK 20
#define MAX_SYS 20

#define MC_MAX 6
// Пусть N -- число задач, для которых можно _доказать_ планируемость формальными методами.
// Сейчас N=6.


struct Task {
    int c;
    int d;
    int u;
};

struct System {
    int n_tasks;
    struct Task tasks[MAX_TASK];
};


struct Group {
    int n_sys;
    struct System systems[MAX_SYS];
};


#endif // STRUCTURES_H
