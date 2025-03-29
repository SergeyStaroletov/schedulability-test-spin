#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "alg.h"
#include "structures.h"
#include "test12.h"
#include "modelcheck.h"

#define DEBUG 1

void debug(char *s) {
    #ifdef DEBUG
    printf("[debug] %s\n", s);
   // va_list args;
   // va_start(args, s);
   // vprintf(s, args);
   // va_end(args);
    #endif
}


bool Test_1(System T) {
    return do_test1(T) == AlgReturn::schedulable;
}


// Как организовано убывание числа процессоров
int change(int m_1, bool Dec) {
    if (Dec) {
        m_1 = floor(m_1 / 2);
    }
    else {
        m_1 = m_1 - 1;
    }
    return m_1;
}


bool tester(System T, int m) {
    if (m == pwr(T) - 1)
        return Test_1(T);
    else
        return ModelChecking(T, m);
}

bool select(System T_1, System T, int m_1, int k) {
    bool safe = false;
    safe = tester(T_1, m_1);
    System T_2 = removeTasks(T, T_1);
    while (!safe && k > 0) {
        if (empty(T_2)) {
            T_2 = removeFirst(removeTasks(T, T_1), m_1 + 2 - k);
            k = k -1;
        }
        replace(T_1, k, head(T_2));
        safe = tester(T_1, m_1);
    }
    return safe;
}

Group MidBin_ET(System T, int m, bool UpDn) {
    int MAX = MC_MAX;
    T = sort(T, Sorting::byU, UpDn);
    Group G_E = newEmptyGroup();
    int n_c = ceil(pwr(T) / MAX);
    int size_c = ceil(m / n_c);
    while (m > 0 && pwr(T) > 0) {
        int m_1 = size_c;
        bool done = false;
        if (pwr(T) < MAX)
            MAX = pwr(T);
        System T_1;
        while (!done && m_1 < MAX && m_1 <= m) {
            T_1 = first(T, MAX);
            done = select(T_1, T, m_1, MAX);
            m_1 = m_1 + 1;
        }
        if (!done)
            return newEmptyGroup();
        G_E = addSystemToGroup(T_1, G_E, m_1);
        T_1 = removeTasks(T, T_1);
        m = m - m_1 + 1;
        if (m_1 - 1 > size_c) {
            Group G_R = MidBin_ET(T, m, UpDn);
            if (G_R.n_sys == 0)
                return newEmptyGroup();
            else
                return addGroupToGroup(G_E, G_R);
        }
    }
    return G_E;
}



bool ModelChecking(System T, int m) {
    int runtime = 0;
    if (compile_spin(T, m)) {
        debug("running verification...");
        AlgReturn r = run_verification(&runtime);
        debug("done run verification");
        return r == AlgReturn::schedulable;
    } else {
        debug("verification compile problem!");
        return false;
    }
}



void tests() {
    System T1 = newEmptySystem();
    T1 = addTask(T1, Task{.c = 1, .d = 1, .u = 1});
    T1 = addTask(T1, Task{.c = 2, .d = 2, .u = 2});
    T1 = addTask(T1, Task{.c = 3, .d = 3, .u = 3});
    printSystem("T1", T1);

    System T2 = newEmptySystem();
    T2 = addTask(T2, Task{.c = 1, .d = 1, .u = 1});
    T2 = addTask(T2, Task{.c = 4, .d = 4, .u = 4});
    T2 = addTask(T2, Task{.c = 3, .d = 3, .u = 3});
    printSystem("T2", T2);

    System T3 = newEmptySystem();
    T3 = addTask(T3, Task{.c = 1, .d = 1, .u = 1});
    T3 = addTask(T3, Task{.c = 3, .d = 3, .u = 3});
    printSystem("T3", T3);

    System T4 = newEmptySystem();
    T4 = addTask(T4, Task{.c = 11, .d = 11, .u = 11});
    T4 = addTask(T4, Task{.c = 1, .d = 1, .u = 1});
    T4 = addTask(T4, Task{.c = 111, .d = 111, .u = 111});
    printSystem("T4",T4);

    printSystem("T1 + T2 = ", addTasks(T1, T2));
    printSystem("T1 - T3 = ", removeTasks(T1, T3));
    T4 = sort(T4, Sorting::byU, true);
    printSystem("Sort(T4) = ", T4);
    printSystem("Remove first 2 of T1 = ", removeFirst(T1, 2));
}


//int main(int argc, char *argv[])
//{
//    tests();
//}
