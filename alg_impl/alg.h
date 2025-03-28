#ifndef ALG_H
#define ALG_H
#include "structures.h"

bool ModelChecking(System T, int m);
Group MinBin_MC (System T, int m, bool UpDn);
Group MidBin_MC(System T, int m);

#endif // ALG_H
