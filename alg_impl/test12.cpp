#include "structures.h"
#include <string>
#include <ecl/ecl.h>

std::string printSystemToLisp(System T) {
    char buf[100];
    std::string result;
    sprintf(buf, "(make-taskset :M %d :N %d :U %f :UC %f :tasks (list ", 0, T.n_tasks, 0, 0);
    result = std::string(buf);

    for (int i = 0; i < T.n_tasks; i++) {
        sprintf(buf, "(make-task :C %d :D %d :T %d) ", T.tasks[i].c, T.tasks[i].d, T.tasks[i].d);
        result += std::string(buf);
    }

    result += std::string("))");

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

void test1(System T) {
    cl_object rez_alg1 = cl_funcall(2, c_string_to_object("Alg1"),
                                    c_string_to_object(printSystemToLisp(T).c_str()));

}
