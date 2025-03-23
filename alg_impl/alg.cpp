#include <stdio.h>
#include <stdlib.h>
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


int cmpByU(const void *a, const void *b) {
    Task *task1 = (Task *)a;
    Task *task2 = (Task *)b;
    return task2->u - task1->u;
}

System sort(System T) {
    qsort(&T.tasks, T.n_tasks, sizeof(Task), &cmpByU);
    return T;
}

int pwr(System T) {
    return T.n_tasks;
}

bool theSameTask(Task t1, Task t2) {
    return (t1.c == t2.c) && (t1.d == t2.d);
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

System replace(System T, int k, Task t) {
    System newSystem = T;
    newSystem.tasks[k] = t;
    return newSystem;
}

void printSystem(System T) {
    printf("{pwr = %d ", T.n_tasks);
    int n = T.n_tasks;
    for (int i = 0; i < n; i++) {
        printf("(c:%d d:%d u:%d)", T.tasks[i].c, T.tasks[i].d, T.tasks[i].u);
        if (i < n - 1) printf(", ");
    }
    printf("}\n");
}

Group newEmptyGroup() {
    Group newG;
    newG.n_sys = 0;
    return newG;
}

Group addSystemToGroup(System T, Group g) {
    g.systems[g.n_sys++] = T;
    return g;
}

Group addGroupToGroup(Group g1, Group g2) {
    Group newG = g1;
    for (int i = 0; i < g2.n_sys; i++) newG.systems[newG.n_sys++] = g2.systems[i];
    return newG;
}

void printGroup(Group g) {
    printf("Set of %d systems = {", g.n_sys);
    for (int i = 0; i < g.n_sys; i++) {
        printf(" System #%d=[\n ", i);
        printSystem(g.systems[i]);
        printf(" ]\n");
    }
    printf("}\n");
}


bool Test_1(System T) {
    return true;
}

// Подбор задач с меньшей утилизацией, если нет планируемости -- Test1
// !!!подменить на версию с мс
int sel_n_test (System T_1, System T_2, System T, int m_1, int k) {
    while (!Test_1(T_1) && k > 0 ) {
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


Group model_checking_ap (System T, int n, int m) { // возвращает разбиение на подсистемы (N, M) c M<N
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
    T = sort(T); 	// сортировка по утилизации
    Group G_1 = newEmptyGroup();
    int m_t = 0; 		// сколько процессоров уже задействовано
    int m_1 = m;		// число процессоров для T_1
    while ((m_1 > 1) || (pwr(T) < m_1 + 1)) {
        int k = m_1 + 1;
        System T_1 = first(T, k);
        System T_2 = removeTasks(T, T_1); //T_2 = T - T_1; //по количеству хвост без T1
        k = sel_n_test(T_1, T_2, T, m_1, k);
        if (k != 0) { // получили ещё одну планируемую подсистему
            m_t = m_t + m_1;
            G_1 = addSystemToGroup(T_1, G_1); //G_1 = G_1 + {T_1}; //добавить множество систем
            T = removeTasks(T, T_1); //T = T - T_1;
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


int main(int argc, char *argv[])
{
    tests();
}
