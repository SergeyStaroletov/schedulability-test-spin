#ifndef TEST12_H
#define TEST12_H

#include "structures.h"

int prepare(int argc, char *argv[]);
AlgReturn do_test1(System T);
AlgReturn do_test2(System T);
void AlgReturn2Str(AlgReturn r, char *buf);
void printSystemToBuf(System T, char *buf);

#endif // TEST12_H
