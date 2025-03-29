#ifndef ALG_H
#define ALG_H
#include "structures.h"

bool ModelChecking(System T, int m);
bool select(System T_1, System T, int m_1, int k);
Group MidBin_ET(System T, int m, bool UpDn);


#endif // ALG_H
