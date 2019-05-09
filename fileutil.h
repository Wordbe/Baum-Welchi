#ifndef FILEUTIL_H_
#define FILEUTIL_H_

#define N_TRANSCRIPTION 1231

#include <vector>
#include <string>
#include "phone.h"

using namespace std;

extern vector<int> phone_seqs[N_TRANSCRIPTION];
extern vector<string> filenames;

//File 경로의 리스트를 생성
int listFilePaths (string dir, vector<string> &paths);

// Read the "trn_mono.txt" file.
void readTranscription();

// Read the "hmm.txt" file.
void ReadIntputFile();

// Write the "hmm.txt" file.
void writeOutputFile();

#endif
