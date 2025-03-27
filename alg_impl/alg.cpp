#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

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

// Подбор задач с меньшей утилизацией, если нет планируемости -- Test1
// !!!подменить на версию с мс
int sel_n_test (System T_1, System T_2, System T, int m_1, int k) {
    while (!Test_1(T_1) && k > 0 ) {
        debug("sel_n_test while");
        if (empty(T_2)) {// заменили k-ю с конца задачу в Т_1 на самую лёгкую из T_2
            T_2 = removeFirst(removeTasks(T, T_1), m_1 + 2 - k); // переходим к следующим задачам, которые после k-й задачи в исходном Т (без самой лёгкой)
            //выкинуть все задачи из T какие есть в T1, количество уменьшается ровно как надо
            k = k - 1; // пробуем заменить очередную задачу с хвоста Т_1
        }
        Task h = head(T_2);
        T_1 = replace(T_1, k, h); // заменяет k-ю задачу из Т_1 на первую задачу из T_2 (T_2 при этом уменьшается)
        T_2 = removeTask(T_2, h); //T2 = T2-head();
    }
    return k;
}

bool model_checking(System T, int n, int m) {
    return false;
}


Group model_checking_ap(System T, int n, int m) { // возвращает разбиение на подсистемы (N, M) c M<N
    if (n > m * MC_MAX) return newEmptyGroup(); // не умеем проверять такое
    T = sort(T); // сортировка по утилизации
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
                safe = model_checking(T_1, MC_MAX, m_1);
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
            G_1 = addSystemToGroup(T_1, G_1);//G_1 = G_1 + {T_1};  // получили ещё одну планируемую подсистему
            T = removeTasks(T, T_1); //T = T - T_1;		// оставшиеся задачи
        }
    }
    //	if G_1 == empty return FAIL;  // нет планируемых подсистем с таким разбиением
    return G_1;
}




Group set_of_syst_div_2(System T, int n, int m) { // возвращает разбиение на подсистемы (x, x-1) и (N, M) c M<N
    debug("sorting...");
    T = sort(T); 	// сортировка по утилизации
    debug("sorting ok");

    Group G_1 = newEmptyGroup();
    int m_t = 0; 		// сколько процессоров уже задействовано
    int m_1 = m;		// число процессоров для T_1
    while ((m_1 > 1) && (pwr(T) > m_1 + 1)) {
        debug("while");
        printf("m1=%d\n", m_1);
        int k = m_1 + 1;
        printf("k=%d\n", k);
        System T_1 = first(T, k);
        debug("first");
        printSystem(T_1);
        System T_2 = removeTasks(T, T_1); //T_2 = T - T_1; //по количеству хвост без T1
        debug("remove");
        printSystem(T_2);
        debug("sel_n_test");
        k = sel_n_test(T_1, T_2, T, m_1, k);
        printf("k=%d\n", k);
        if (k != 0) { // получили ещё одну планируемую подсистему
            m_t = m_t + m_1;
            G_1 = addSystemToGroup(T_1, G_1); //G_1 = G_1 + {T_1}; //добавить множество систем
            T = removeTasks(T, T_1); //T = T - T_1;
            debug("removeTasks");
            printSystem(T);
        } else {
            m_1 = floor(m_1 / 2);
        }
    }
    // Теперь модел чекинг для оставшихся в Т
    Group G_2 = model_checking_ap(T, pwr(T), m - m_t);
    //if (emptyG(G_2)) return newEmptySystem();
    return addGroupToGroup(G_1, G_2); //G_1 + G_2
}


void tests() {
    System T1 = newEmptySystem();
    T1 = addTask(T1, Task{.c = 1, .d = 1, .u = 1});
    T1 = addTask(T1, Task{.c = 2, .d = 2, .u = 2});
    T1 = addTask(T1, Task{.c = 3, .d = 3, .u = 3});
    printf("T1 = ");
    printSystem(T1);

    System T2 = newEmptySystem();
    T2 = addTask(T2, Task{.c = 1, .d = 1, .u = 1});
    T2 = addTask(T2, Task{.c = 4, .d = 4, .u = 4});
    T2 = addTask(T2, Task{.c = 3, .d = 3, .u = 3});
    printf("T2 = ");
    printSystem(T2);

    System T3 = newEmptySystem();
    T3 = addTask(T3, Task{.c = 1, .d = 1, .u = 1});
    T3 = addTask(T3, Task{.c = 3, .d = 3, .u = 3});
    printf("T3 = ");
    printSystem(T3);

    System T4 = newEmptySystem();
    T4 = addTask(T4, Task{.c = 11, .d = 11, .u = 11});
    T4 = addTask(T4, Task{.c = 1, .d = 1, .u = 1});
    T4 = addTask(T4, Task{.c = 111, .d = 111, .u = 111});
    printf("T4 = ");
    printSystem(T4);

    printf("T1 + T2 = ");
    printSystem(addTasks(T1, T2));
    printf("T1 - T3 = ");
    printSystem(removeTasks(T1, T3));
    printf("Sort(T4) = ");
    T4 = sort(T4);
    printSystem(T4);
    printf("Remove first 2 of T1 = ");
    printSystem(removeFirst(T1, 2));
}


//int main(int argc, char *argv[])
//{
//    tests();
//}
