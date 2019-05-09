#pragma once
#ifndef TRANSITION_H_
#define TRANSITION_H_

#include "phone.h"
#include "fileutil.h"
#include <string>
#include <vector>

#define MAX_PHONE_SEQS 43 // sil ( s eh v ah n sp )*6 (s eh v ah n) sil 나열시 최대 ( 1+36+5+1 = 43 )

using namespace std;

struct transitionType {
	int to_phone;
	int to_state;
	double to_prob;
};

struct transitionPrevType {
	int from_phone;
	int from_state;
	double from_prob;
};

// transitions from beginning
extern vector<transitionType> beginningTransitions;

// transitions from each states to next
extern vector<transitionType> transitions[MAX_PHONE_SEQS][N_STATE];

// transitions from each states to prev
extern vector<transitionPrevType> transitionsPrev[MAX_PHONE_SEQS][N_STATE];

// transitions to Ending
extern vector<transitionPrevType> endingTransitions;

// initialize and set beginningTransitions and transitions
void initTthTransitions(int tau);

#endif