#ifndef ACCUMULATION_H_
#define ACCUMULATION_H_

#include "transition.h"
#include "observation.h"
#include "fileutil.h"

#define MAX_TIME_LENGTH 1000

struct valueType {
	bool isAssigned;
	double prob;
};

extern valueType alphas[MAX_TIME_LENGTH][MAX_PHONE_SEQS][N_STATE];
extern valueType betas[MAX_TIME_LENGTH][MAX_PHONE_SEQS][N_STATE];

void runAcculmulation(int length, double spectrogram[][N_DIMENSION], int tau);

#endif