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




Group MaxBin_T(System T, int m, int UpDn, bool Dec) {
    T = sort(T, Sorting::byU, UpDn);
    Group G_A = newEmptyGroup();
    int m_1 = m - 1;
    while (m_1 > 1 && pwr(T) >= m_1 + 1 && m > m_1) {
        System T_1 = first(T, m_1 + 1);
        bool done = select(T_1, T, m_1, m_1 + 1);
        if (done) {
            G_A = addSystemToGroup(T_1, G_A, m_1);
            T = removeTasks(T, T_1);
            m = m - m_1;
        } else {
            if (Dec)
                m_1 = ceil(m_1 / 2);
            else
                m_1 = m_1 - 1;
        }
    }
    if (pwr(T) <= m)
        return addSystemToGroup(T, G_A, m);
    Group G_E = ExactTestA(T, m);
    if (G_E.n_sys == 0) return newEmptyGroup();
    return addGroupToGroup(G_A, G_E);
}


Group MidBin_T(System T, int m, bool UpDn) {
    T = sort(T, Sorting::byU, UpDn);
    Group G_A = newEmptyGroup();
    int size_c = ceil(sqrt(m));
    while (m > 1 && pwr(T) > 0) {
        double m_1 = size_c;
        if (pwr(T) <= m_1) return addSystemToGroup(T, G_A, m_1);
        bool done = false;
        System T_1;
        while (!done && m_1 > 0) {
            T_1 = first(T, m_1 + 1);
            done = select(T_1, T, m_1, m_1 + 1);
            m_1 = m_1 - 1;
        }
        if (done) {
            m = m - m_1 - 1;
            G_A = addSystemToGroup(T_1, G_A, m_1);
            T = removeTasks(T, T_1);
        } else break;
    }
    if (pwr(T) == 0) return G_A;
    if (pwr(T) <= m) return addSystemToGroup(T, G_A, m);
    Group G_E = ExactTestA(T, m);
    if (G_E.n_sys == 0) return newEmptyGroup();
    return addGroupToGroup(G_A, G_E);
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

Group MinBin_ET(System T, int m, bool UpDn) {
    int MAX = MC_MAX;
    T = sort(T, Sorting::byU, UpDn);
    Group G_E = newEmptyGroup();
    while (m > 0 && pwr(T) > 0) {
        int m_1 = 1;
        bool done = false;
        if (pwr(T) < MAX) MAX = pwr(T);
        System T_1;
        while (!done && m_1 < MAX && m_1 <= m) {
            T_1 = first(T, MAX);
            done = select(T_1, T, m_1, MAX);
            m_1 = m_1 + 1;
        }
        if (!done) return newEmptyGroup();
        G_E = addSystemToGroup(T_1, G_E, m_1);
        T = removeTasks(T, T_1);
        m = m - m_1 + 1;
    }
    return G_E;
}

Group Assignment(System T, int m) {
    Group G = newEmptyGroup();
    G = Choose_UpDn(T, m, Alg::Alg_MaxBin_T);
    if (G.n_sys != 0) return G;
    G = Choose_UpDn(T, m, Alg::Alg_MidBin_T);
    if (G.n_sys != 0) return G;
    G = Choose_UpDn(T, m, Alg::Alg_MidBin_ET);
    if (G.n_sys != 0) return G;
    G = Choose_UpDn(T, m, Alg::Alg_MinBin_ET);
    if (G.n_sys != 0) return G;
    return newEmptyGroup();
}


Group RunA(System T, int m, Alg A, bool UpDn) {
    Group G = newEmptyGroup();
    if (A == Alg_MaxBin_T) G = MaxBin_T(T, m, UpDn, true);
    if (G.n_sys != 0) return G;
    if (A == Alg_MaxBin_T) G = MaxBin_T(T, m, UpDn, false);
    if (A == Alg_MidBin_T) G = MidBin_T(T, m, UpDn);
    if (A == Alg_MidBin_ET) G = MidBin_ET(T, m, UpDn);
    if (A == Alg_MinBin_ET) G = MinBin_ET(T, m, UpDn);
    return G;
}

Group Choose_UpDn(System T, int m, Alg A) {
    Group G = newEmptyGroup();
    G = RunA(T, m, A, true);
    if (G.n_sys != 0) return G;
    G = RunA(T, m, A, false);
    return G;
}

Group ExactTestA(System T, int m) {
    Group G_E = MidBin_ET(T, m, true);
    if (G_E.n_sys != 0) return G_E;
    G_E = MidBin_ET(T, m, false);
    if (G_E.n_sys != 0) return G_E;
    G_E = MinBin_ET(T, m, true);
    if (G_E.n_sys != 0) return G_E;
    G_E = MinBin_ET(T, m, false);
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
