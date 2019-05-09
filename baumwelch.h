#ifndef BAUMWELCH_H_
#define BAUMWELCH_H_

#include "accumulation.h"

#define MAX_TIME_LENGTH 1000 

extern double firstGamma[N_PHONE][N_STATE];
extern double lastGamma[N_PHONE][N_STATE];
extern double sumGamma[N_PHONE][N_STATE];
extern double sumGammaForKsi[N_PHONE][N_STATE];
extern double ksi[N_PHONE][N_STATE][N_STATE];
extern double sumGammaK[N_PHONE][N_STATE][N_PDF];
extern double means[N_PHONE][N_STATE][N_PDF][N_DIMENSION];
extern double vars[N_PHONE][N_STATE][N_PDF][N_DIMENSION];

extern phoneType newPhones[N_PHONE];

void runBaumWelch();

#endif