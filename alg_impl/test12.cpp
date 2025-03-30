#include "structures.h"
#include <string>
#include <string.h>
#include <ecl/ecl.h>

std::string printSystemToLisp(System T) {
    char buf[300];
    std::string result;
    sprintf(buf, "(make-taskset :M %d :N %d :U 0 :UC 0 :tasks (list ", 0, T.n_tasks);//, 0, 0);
    result = std::string(buf);

    for (int i = 0; i < T.n_tasks; i++) {
        sprintf(buf, "(make-task :C %d :D %d :T %d) ", T.tasks[i].c, T.tasks[i].d, T.tasks[i].d);
        result += std::string(buf);
    }

    result += std::string(") )");
    return result;
}

int prepare(int argc, char *argv[]) {
    int b = cl_boot(argc, argv);
    if (!b) {
        printf("ECL is not loaded!\n");
        return 1;
    }

    cl_object result = cl_eval(c_string_to_object("(defstruct (task) C D T)"));
    result = cl_eval(c_string_to_object("(defstruct (taskset) M N U UC tasks)"));

    cl_eval(c_string_to_object("(defun implies (x y)"
                               "(cond ((equal x nil) T)"
                               "(T y)))"));

    cl_eval(c_string_to_object(
        "(defun Alg1 (ts)"
        "  (do ((n (taskset-N ts))"
        "       (tasks (taskset-tasks ts))"
        "       (i 1 (+ i 1)))"
        "      ((> i n))"
        "    (do ((j 1 (+ j 1)))"
        "	((> j n) (return-from Alg1 'infeasible))"
        "      (let ((Ci (task-C (nth (- i 1) tasks)))"
        "	    (Cj (task-C (nth (- j 1) tasks)))"
        "	    (Di (task-D (nth (- i 1) tasks))))"
        "	(if (not (implies"
        "		  (not (equal i j))"
        "		  (or (< Di Cj) (< Di (+ Cj Ci)))))"
        "	    (return))))"
        "    (do ((j 1 (+ j 1)))"
        "	((>= j i))"
        "      (let ((Ci (task-C (nth (- i 1) tasks)))"
        "	    (Cj (task-C (nth (- j 1) tasks)))"
        "	    (Di (task-D (nth (- i 1) tasks)))"
        "	    (Dj (task-D (nth (- j 1) tasks))))"
        "	(if (and"
        "	     (or"
        "	      (and (<= Cj Di) (< Di (* 2 Cj)))"
        "	      (and (<= (+ Cj Ci) Di) (< Di (+ (* 2 Cj) Ci))))"
        "	     (> Ci (- Dj Cj)))"
        "	    (return-from Alg1 'infeasible)))))"
        "  (return-from Alg1 'schedulable))"
        ));


    cl_eval(c_string_to_object(
        "(defun Alg2 (ts)"
        "  (do ((n (taskset-N ts))"
        "       (tasks (taskset-tasks ts))"
        "       (i 1 (+ i 1)))"
        "      ((> i n))"
        "    (do ((j 1 (+ j 1)))"
        "	((> j n) (return-from Alg2 'infeasible))"
        "      (let ((Cj (task-C (nth (- j 1) tasks)))"
        "	    (Di (task-D (nth (- i 1) tasks)))"
        "       (Ci (task-C (nth (- i 1) tasks))))"
        "	(if (not (implies"
        "		  (not (equal i j))"
        "		  (or (< Di Cj) (< Di (+ Cj Ci)))))"
        "	    (return))))"
        "    (do ((j 1 (+ j 1)))"
        "	((>= j i))"
        "      (let ((Ci (task-C (nth (- i 1) tasks)))"
        "	    (Cj (task-C (nth (- j 1) tasks)))"
        "	    (Di (task-D (nth (- i 1) tasks)))"
        "	    (Dj (task-D (nth (- j 1) tasks))))"
        "	(if (and"
        "	     (or"
        "	      (and (<= Cj Di) (< Di (* 2 Cj)))"
        "	      (and (<= (+ Cj Ci) Di) (< Di (+ (* 2 Cj) Ci))))"
        "	     (> Ci (- Dj Cj)))"
        "	    (return-from Alg2 'infeasible)))))"
        "  (return-from Alg2 'unknown))"
        ));

    return 0;
}

AlgReturn do_test1(System T) {
    cl_object rez_alg1 = cl_funcall(2, c_string_to_object("Alg1"),
                                    cl_eval(c_string_to_object(printSystemToLisp(T).c_str())));

    if (!strcmp((const char *)rez_alg1->symbol.name->string.self, "SCHEDULABLE"))
        return AlgReturn::schedulable;
    if (!strcmp((const char *)rez_alg1->symbol.name->string.self, "INFEASIBLE"))
        return AlgReturn::infeasible;

    return AlgReturn::unknown;

}

AlgReturn do_test2(System T) {
    cl_object rez_alg2 = cl_funcall(2, c_string_to_object("Alg2"),
                                    cl_eval(c_string_to_object(printSystemToLisp(T).c_str())));

    if (!strcmp((const char *)rez_alg2->symbol.name->string.self, "SCHEDULABLE"))
        return AlgReturn::schedulable;
    if (!strcmp((const char *)rez_alg2->symbol.name->string.self, "INFEASIBLE"))
        return AlgReturn::infeasible;

    return AlgReturn::unknown;

}

void AlgReturn2Str(AlgReturn r, char *buf) {
    if (r == AlgReturn::schedulable) strcpy(buf, "schedulable"); else
    if (r == AlgReturn::unknown) strcpy(buf, "unknown"); else
        if (r == AlgReturn::infeasible) strcpy(buf, "infeasible"); else
    strcpy(buf, "?");
}



