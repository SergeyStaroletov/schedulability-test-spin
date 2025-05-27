#ifndef ALG_H
#define ALG_H
#include "structures.h"

bool ModelChecking(System T, int m);

Group MaxBin_T(System T, int m, UpDown UpDn, bool Dec); //alg1
Group MidBin_T(System T, int m, UpDown UpDn); //alg2
Group MidBin_ET(System T, int m, UpDown UpDn); //alg3
Group MinBin_ET(System T, int m, UpDown UpDn); //alg4

System select(System T, int m_1, int k);

Group Assignment(System T, int m); //alg6
Group Choose_UpDn(System T, int m, Alg A);
Group RunA(System T, int m, Alg A, UpDown UpDn); //alg8

Group ExactTestA(System T, int m); //alg9


#endif // ALG_H
