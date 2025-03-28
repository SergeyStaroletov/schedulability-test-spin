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


// Подбор задач с мЕньшей утилизацией, если нет планируемости
//todo: роль T?
int select(System T_1, System T_2, System T, int m_1, int k) {
    bool safe = false;

    if (m_1 == pwr(T_1))
        safe = Test_1(T_1);
    else
        safe = ModelChecking(T_1, m_1);

    while (!safe && k > 0) {
        if (empty(T_2)) {// заменили k-ю с конца задачу в Т_1 на самую лёгкую из T_2
            T_2 = removeFirst(removeTasks(T, T_1), m_1 + 2 - k); // переходим к следующим задачам, которые после k-й задачи в исходном Т (без самой лёгкой)
            k = k - 1; // пробуем заменить очередную задачу с хвоста Т_1
        }
        Task h = head(T_2);
        replace(T_1, k, h); // заменяет k-ю задачу из Т_1 на первую задачу из T_2 (T_2 при этом уменьшается)
        T_2 = removeTask(T_2, h);

        if(m_1 == MC_MAX - 1)
            safe = Test_1(T_1);
        else
            safe = ModelChecking(T_1, m_1);
    }
    return k;
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


// Алгоритм назначения только через формальные методы 1
// Можно начинать с проверки самых толстых на минимальном числе процессоров
// Пусть MC_MAX -- число задач, для которых можно _доказать_ планируемость формальными методами. Сейчас N=6.

Group MinBin_MC (System T, int m, bool UpDn) { // возвращает разбиение на подсистемы (N, M) c M<N
    if (pwr(T) > m * MC_MAX) return newEmptyGroup(); // не умеем проверять такое
    T = sort(T, Sorting::byU, UpDn); // сортировка по утилизации
    Group G_M = newEmptyGroup();
    int m_t = 1; 		// сколько процессоров уже задействовано
    while (pwr(T) > 0 && m_t < m) { // pwr(T) -- число задач в Т
        debug("while");
        System T_1 = first(T, MC_MAX);
        printSystem("T1", T_1);
        int m_1 = 1;
        int k = MC_MAX;
        System T_2 = removeTasks(T, T_1);//T_2 = T - T_1;
        printSystem("T2", T_2);
        bool safe;
        while (k > 0) {
            while (m_1 < MC_MAX && m_t < m) {
                safe = ModelChecking(T_1, m_1);
                printf("[MC] m_1=%d m_t=%d safe=%d\n", m_1, m_t, safe);
                if (safe) { break; }
                m_1++;
                m_t++;
            }
            if (!safe && m_t == m) return newEmptyGroup();
            if (m_1 < MC_MAX) break; // T_1 планируема на m_1-1 процессоре
            // если не так, меняем задачи на полегче и пробуем ещё
            if (empty(T_2)) {// заменили k-ю с конца задачу в Т_1 на самую лёгкую из T_2
                T_2 = removeFirst(removeTasks(T, T_1), m_1 + 2 - k); // переходим к следующим задачам, которые после k-й задачи в исходном Т (без самой лёгкой)
                printSystem("T2 after removeFirst", T_2);
                k = k - 1; // пробуем заменить очередную задачу с хвоста Т_1
                debug("empty t_2 -> T_2 = removeFirst(removeTasks(T, T_1), m_1 + 2 - k)");
                printf("k = %d \n", k);
            }
           // replace(T_1, k, head(T_2)); // заменяет k-ю задачу из Т_1 на первую задачу из T_2 (T_2 при этом уменьшается)
            Task h = head(T_2);
            T_1 = replace(T_1, k, h); // заменяет k-ю задачу из Т_1 на первую задачу из T_2 (T_2 при этом уменьшается)
            T_2 = removeTask(T_2, h); //T2 = T2-head();
            debug("replace(T_1, k, head(T_2))");
            printSystem("T1 after replace", T_1);
            printSystem("T2 after remove head task", T_2);
            m_t = m_t - MC_MAX + 1;
            m_1 = 1;
            printf("new m_t = %d, m_1 = 1\n", m_t);
        }
        if (k == 0) { // не получилось запланировать N задач
            return newEmptyGroup();
        }
        else {
            G_M = addSystemToGroup(T_1, G_M, m_1);//G_1 = G_1 + {T_1};  // получили ещё одну планируемую подсистему
            debug("GM is updated!");

            T = removeTasks(T, T_1); //T = T - T_1;		// оставшиеся задачи
            printSystem("T after removeTasks from T1", T);
        }
    }
    //	if G_1 == empty return FAIL;  // нет планируемых подсистем с таким разбиением
    return G_M;
}



Group MidBin_MC(System T, int m) { // возвращает равномерное разбиение на подсистемы (N, M) c M<N
    if (pwr(T) > m * MC_MAX) return newEmptyGroup(); // не умеем проверять такое
    T = sort(T, Sorting::byU, true); // сортировка по утилизации
    Group G_M = newEmptyGroup();
    int n_c = ceil(T.n_tasks / MC_MAX); // число кластеров
    int m_1 = ceil(m / n_c); // число процессоров
    int m_t = 0;
    while(m_t < m) {
        System T_1 = first(T, MC_MAX);
        System T_2 = removeTasks(T, T_1);
        int done = 0;
        while (done == 0 && m_1 < MC_MAX) {
            done = select(T_1, T_2, T, m_1, MC_MAX);
            m_1++;
        }
        if (done == 0)
            return newEmptyGroup(); // не получилось запланировать N задач
        G_M = addSystemToGroup(T_1, G_M, m_1); //G_M = G_M + {T_1}  // получили ещё одну планируемую подсистему
        T = removeTasks(T, T_1); //T = T - T_1;		// оставшиеся задачи
        m_t = m_t + m_1 - 1;
        if (m_1 - 1 > ceil(m / n_c)) { // потратили процессоров больше, чем хотели
            Group G_R = MidBin_MC(T, m - m_t);
            if (G_R.systems == 0)
                return newEmptyGroup();
            else return addGroupToGroup(G_M, G_R); //G_M + G_R
        }
        if (m - m_t - m_1 < 0)
        {
            m_1 = m - m_t;
        }
    }
    return G_M;
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


Group model_checking_ap(System T, int n, int m) { // возвращает разбиение на подсистемы (N, M) c M<N
    if (n > m * MC_MAX) return newEmptyGroup(); // не умеем проверять такое
    T = sort(T, Sorting::byU, true); // сортировка по утилизации
    Group G_1 = newEmptyGroup();
    int m_t = 1; 		// сколько процессоров уже задействовано
    while (pwr(T) > MC_MAX && m_t < m) { // pwr(T) -- число задач в Т
        System T_1 = first(T, MC_MAX);
        int m_1 = 1;
        int k = MC_MAX;
        System T_2 = removeTasks(T, T_1);//T_2 = T - T_1;
        bool safe;
        while (k > 0) {
            while (m_1 < MC_MAX && m_t < m) {
                safe = ModelChecking(T_1, m_1);
                if (safe) { break; }
                m_1++;
                m_t++;
            }
            if (!safe && m_t == m) return newEmptyGroup();
            if (m_1 < MC_MAX) break; // T_1 планируема на m_1-1 процессоре
            // если не так, меняем задачи на полегче и пробуем ещё
            if (empty(T_2)) {// заменили k-ю с конца задачу в Т_1 на самую лёгкую из T_2
                T_2 = removeFirst(removeTasks(T, T_1), m_1 + 2 - k); // переходим к следующим задачам, которые после k-й задачи в исходном Т (без самой лёгкой)
                k = k - 1; // пробуем заменить очередную задачу с хвоста Т_1
            }
            replace(T_1, k, head(T_2)); // заменяет k-ю задачу из Т_1 на первую задачу из T_2 (T_2 при этом уменьшается)
            m_t = m_t - MC_MAX + 1;
            m_1 = 1;
        }
        if (k == 0) { // не получилось запланировать N задач
            return newEmptyGroup();
        }
        else {
            G_1 = addSystemToGroup(T_1, G_1, m_1);//G_1 = G_1 + {T_1};  // получили ещё одну планируемую подсистему
            T = removeTasks(T, T_1); //T = T - T_1;		// оставшиеся задачи
        }
    }
    //	if G_1 == empty return FAIL;  // нет планируемых подсистем с таким разбиением
    return G_1;
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
