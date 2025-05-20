#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>


#include "alg.h"
#include "structures.h"
#include "test12.h"
#include "modelcheck.h"

#define DEBUG 1
#define EXPERIMENT 1
//#define NEED_MID_BIN 1
//#define NEED_MIN_BIN 1



extern FILE *csv;
extern float current_u;
extern float current_uc;
extern char buf_test1[50];
extern char buf_test2[50];
extern char buf_sys[2000];


void debug(char *s,...) {
    #ifdef DEBUG
    printf("[debug] ");
    va_list args;
    va_start(args, s);
    vprintf(s, args);
    va_end(args);
    printf("\n");
    #endif
}


bool Test_1(System T) {
    sort(T, Sorting::byP, true);
    return do_test1(T) == AlgReturn::schedulable;
}


AlgReturn ES_Test(System T, int m) {
    bool retFalse = (Test_1(T) == AlgReturn::infeasible);
    if (retFalse) return AlgReturn::infeasible;
    if (m == pwr(T)) return AlgReturn::schedulable;
    return AlgReturn::unknown;
}


bool tester(System T, int m) {
    if (m == pwr(T) - 1)
        return Test_1(T);
    else
        return ModelChecking(T, m);
}

System select(System T, int m_1, int k) {
    debug("select");
    System T_1 = first(T, k);
    bool safe = tester(T_1, m_1);
    System T_2 = removeTasks(T, T_1);
    if (empty(T_2) && !safe)
        return newEmptySystem();
    System T_3 = T_2;
    while (!safe) {
        if (empty(T_2)) {
            debug("k -> %d\n", k);
            if (k == 1 || pwr(T_3) == 1) return newEmptySystem();
            T_2 = removeLast(T_3);
            T_3 = T_2;
            k = k - 1;
        }
        Task h = head(T_2);
        T_1 = replace(T_1, k, h);
        T_2 = removeTask(T_2, h);
        safe = tester(T_1, m_1);
    }
    if (safe) return T_1;//fix
    return newEmptySystem();
}




Group MaxBin_T(System T, int m, bool UpDn, bool Dec) {
    debug("----> MaxBin_T for n = %d m = %d UpDn = %d Dec = %d <----", pwr(T), m, UpDn, Dec);
    T = sort(T, Sorting::byU, UpDn);
    Group G_A = newEmptyGroup();
    int m_1 = (m * MC_MAX - pwr(T) + 1) / (MC_MAX - 1);//m - 1;
    debug("m1 = %d", m_1);
    while (m_1 > 1 && m > m_1) {
        System T_1 = select(T, m_1, m_1 + 1);
        if (!empty(T_1)) {
            debug(" ---> done !");
            G_A = addSystemToGroup(T_1, G_A, m_1);
            T = removeTasks(T, T_1);
            if (empty(T)) return G_A;
            m = m - m_1;
            debug("new m = %d", m);
            if (pwr(T) <= m) return addSystemToGroup(T, G_A, m);
            m_1 = (m * MC_MAX - pwr(T) + 1) / (MC_MAX - 1);
        } else {
            if (Dec)
                m_1 = floor(m_1 / 2);
            else
                m_1 = m_1 - 1;
            debug("new m1 = %d", m_1);
        }
    }
    Group G_E = ExactTestA(T, m);
    if (G_E.n_sys == 0) return newEmptyGroup();
    return addGroupToGroup(G_A, G_E);
}


Group MidBin_T(System T, int m, bool UpDn) {
    debug("----> MidBin_T for n = %d m = %d UpDn = %d <----", pwr(T), m, UpDn);
    T = sort(T, Sorting::byU, UpDn);
    Group G_A = newEmptyGroup();
    int size_c = floor(sqrt(m));
    while (m > 1) {
        double m_1 = size_c;
        if (m <= m_1) m_1 = m - 1;
        System T_1 = newEmptySystem();
        while (empty(T_1) && m_1 > 0) {
            T_1 = select(T, m_1, m_1 + 1);
            m_1 = m_1 - 1;
        }
        if (!empty(T_1)) {
            debug("! ---> done !");
            G_A = addSystemToGroup(T_1, G_A, m_1 + 1);//
            T = removeTasks(T, T_1);
            if (empty(T)) return G_A;
            m = m - m_1 - 1;
            debug("new m = %d", m);
            if (pwr(T) <= m) return addSystemToGroup(T, G_A, m);
        } else break;
    }
    Group G_E = ExactTestA(T, m);
    if (G_E.n_sys == 0) return newEmptyGroup();
    return addGroupToGroup(G_A, G_E);
}


Group MidBin_ET(System T, int m, bool UpDn) {
    debug("----> MidBin_ET for n = %d m = %d UpDn = %d <----", pwr(T), m, UpDn);
    int MAX = MC_MAX;
    T = sort(T, Sorting::byU, UpDn);
    Group G_E = newEmptyGroup();
    int size_c = floor(1.0 * m * MAX / pwr(T));
    while (m > 0) {
        if (pwr(T) < MAX) MAX = pwr(T);
        int m_1 = size_c;
        if (m <= m_1) m_1 = m;
        System T_1 = newEmptySystem();
        while (empty(T_1) && m_1 < MAX && m_1 <= m) {
            T_1 = select(T, m_1, MAX);
            m_1 = m_1 + 1;
        }
        if (empty(T_1)) return newEmptyGroup();
        G_E = addSystemToGroup(T_1, G_E, m_1 - 1);
        T = removeTasks(T, T_1);
        if (empty(T)) return G_E;
        m = m - m_1 + 1;
        if (pwr(T) <= m) return addSystemToGroup(T, G_E, m);
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

Group MinBin_ET_small(System T, int m, bool UpDn) {
    debug("----> MinBin_ET_small for n = %d m = %d UpDn = %d <----", pwr(T), m, UpDn);
    int MAX = MC_MAX;
    if (T.n_tasks / MAX > m)
        return newEmptyGroup();
    T = sort(T, Sorting::byU, UpDn);
    Group G_E = newEmptyGroup();
    while (m > 0) {
        int m_1 = 1;
        if (pwr(T) < MAX) MAX = pwr(T);
        System T_1 = newEmptySystem();
        while (empty(T_1) && m_1 < MAX && m_1 <= m) {
            T_1 = select(T, m_1, MAX);
            m_1 = m_1 + 1;
        }
        if (empty(T_1)) return newEmptyGroup();
        G_E = addSystemToGroup(T_1, G_E, m_1 - 1);
        T = removeTasks(T, T_1);
        if (empty(T)) return G_E;
        m = m - m_1 + 1;
        if (m == 0 && pwr(T) > 0)
            return newEmptyGroup();
        //if (pwr(T) <= m) return addSystemToGroup(T, G_E, m);
    }
    return G_E;
}


Group MinBin_ET(System T, int m, bool UpDn) {
    debug("----> MinBin_ET for n = %d m = %d UpDn = %d <----", pwr(T), m, UpDn);
    int MAX = MC_MAX;
    T = sort(T, Sorting::byU, UpDn);
    Group G_E = newEmptyGroup();
    while (m > 0) {
        int m_1 = 1;
        if (pwr(T) < MAX) MAX = pwr(T);
        System T_1 = newEmptySystem();
        while (empty(T_1) && m_1 < MAX && m_1 <= m) {
            T_1 = select(T, m_1, MAX);
            m_1 = m_1 + 1;
        }
        if (empty(T_1)) return newEmptyGroup();
        G_E = addSystemToGroup(T_1, G_E, m_1 - 1);
        T = removeTasks(T, T_1);
        if (empty(T)) return G_E;
        m = m - m_1 + 1;
        if (pwr(T) <= m) return addSystemToGroup(T, G_E, m);
    }
    return G_E;
}


Group Assignment(System T, int m) {
    debug("----> Assignment for n = %d m = %d <----", pwr(T), m);
    Group G = newEmptyGroup();
    int as = 1;
#ifdef NEED_MIN_BIN
    debug("Assignment %d", as);
    G = Choose_UpDn(T, m, Alg::Alg_MinBin_ET);
    as++;
    #ifndef EXPERIMENT
    if (G.n_sys != 0) return G;
    #endif
#endif

#ifdef NEED_MID_BIN
    debug("Assignment %d", as);
    G = Choose_UpDn(T, m, Alg::Alg_MidBin_T);
    as++;
    #ifndef EXPERIMENT
    if (G.n_sys != 0) return G;
    #endif
    debug("Assignment %d", as);
    G = Choose_UpDn(T, m, Alg::Alg_MidBin_ET);
    as++;
    #ifndef EXPERIMENT
    if (G.n_sys != 0) return G;
    #endif
#else
    debug("Assignment %d", as);
    G = Choose_UpDn(T, m, Alg::Alg_MinBin_ET_small);
    as++;
    #ifndef EXPERIMENT
    if (G.n_sys != 0) return G;
    #endif
#endif

    //debug("Assignment %d", as);
    //G = Choose_UpDn(T, m, Alg::Alg_MaxBin_T);

    //if (G.n_sys != 0) return G;
    return newEmptyGroup();
}


void SaveCVS(Alg A, bool UpDn, bool Dec, System T, int m, Group G, double run_time) {
    char buf_algname[50];
    if (A == Alg::Alg_MaxBin_T) strcpy(buf_algname, "MaxBin_T");
    else if (A == Alg::Alg_MidBin_T) strcpy(buf_algname, "MidBin_T");
    else if (A == Alg::Alg_MidBin_ET) strcpy(buf_algname, "MidBin_ET");
    else if (A == Alg::Alg_MinBin_ET) strcpy(buf_algname, "MinBin_ET");
    else if (A == Alg::Alg_MinBin_ET_small) strcpy(buf_algname, "MinBin_ET_small");


    if (A == Alg::Alg_MaxBin_T) {
        if (Dec)
            strcat(buf_algname, "Dec2");
        else
            strcat(buf_algname, "Dec-");
    }

    if (UpDn)
        strcat(buf_algname, "Up");
    else
        strcat(buf_algname, "Down");

    char buf_grp[2000];
    char buf_grp_l[1000];

    printGroupToBuf(G, buf_grp);
    int cpus = printGroupToBufLess(G, buf_grp_l);

    bool I = (ES_Test(T, m) == false);
    bool C = (G.n_sys != 0);

   // fprintf(csv, "id;n;m;u;uc;test1;test2;C?;I;alg;group_count;runtime;assign;system;group\n");

    fprintf(csv, "%d;%d;%d;%f;%f;%s;%s;%s;%s;%s;%d;%d;\"[AllCPUs=%d]%s\";\"%s\";\"%s\"\n",
        T.id,
        T.n_tasks,
        m,
        current_u,
        current_uc,
        buf_test1,
        buf_test2,
        C ? "true" : "false",
        I ? "true" : "false",
        buf_algname,
        G.n_sys,
        (int)run_time,
        cpus,
        buf_grp_l,
        buf_sys,
        buf_grp
    );
    fflush(csv);

#ifdef DEBUG
    if (G.n_sys != 0) {
        debug("Alg %s FOUND assignment!", buf_algname);
        printGroup(G);
    } else {
        debug("Alg %s FAILED assignment!", buf_algname);
    }
#endif

}



Group RunA(System T, int m, Alg A, bool UpDn) {
    struct timeval  tv1, tv2;

    Group G = newEmptyGroup();
    if (A == Alg_MaxBin_T) {
        gettimeofday(&tv1, NULL);
        G = MaxBin_T(T, m, UpDn, true);
        gettimeofday(&tv2, NULL);

        #ifdef EXPERIMENT
        SaveCVS(A, UpDn, true, T, m, G, (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));
        #endif

        #ifndef EXPERIMENT
        if (G.n_sys != 0) return G;
        #endif
    }

    gettimeofday(&tv1, NULL);
    if (A == Alg_MaxBin_T) G = MaxBin_T(T, m, UpDn, false);
    if (A == Alg_MidBin_T) G = MidBin_T(T, m, UpDn);
    if (A == Alg_MidBin_ET) G = MidBin_ET(T, m, UpDn);
    if (A == Alg_MinBin_ET) G = MinBin_ET(T, m, UpDn);
    if (A == Alg_MinBin_ET_small) G = MinBin_ET_small(T, m, UpDn);
    gettimeofday(&tv2, NULL);

    #ifdef EXPERIMENT
    SaveCVS(A, UpDn, false, T, m, G, (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));
    #endif
    return G;
}

Group Choose_UpDn(System T, int m, Alg A) {
    debug("----> Choose_UpDn for n = %d m = %d <----", pwr(T), m);
    Group G = newEmptyGroup();
    G = RunA(T, m, A, true);
    #ifndef EXPERIMENT
    if (G.n_sys != 0) return G;
    #endif
    G = RunA(T, m, A, false);
    return G;
}

Group ExactTestA(System T, int m) {
    debug("----> ExactTestA for n = %d m = %d <----", pwr(T), m);
    Group G_E;
#ifndef EXPERIMENT
    debug("ExactTestA 1/4");
    G_E = MidBin_ET(T, m, true);
    if (G_E.n_sys != 0) return G_E;

    debug("ExactTestA 2/4");
    G_E = MidBin_ET(T, m, false);
    if (G_E.n_sys != 0) return G_E;
#endif
    debug("ExactTestA 3/4");
    G_E = MinBin_ET_small(T, m, true);
    if (G_E.n_sys != 0) return G_E;

    debug("ExactTestA 4/4");
    G_E = MinBin_ET_small(T, m, false);
    return G_E;
}



bool ModelChecking(System T, int m) {
    char buf_v[50];

    sort(T, Sorting::byP, true);
    int runtime = 0;
    if (compile_spin(T, m)) {
        debug("running verification...");
        AlgReturn r = run_verification(&runtime);
        AlgReturn2Str(r, buf_v);
        debug("done run verification");
        debug(buf_v);
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
