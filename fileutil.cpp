#define _CRT_SECURE_NO_WARNINGS
#include <sys/types.h>
#include <string.h>
#include <iostream>
#include "fileutil.h"
#include "baumwelch.h"

using namespace std;

vector<int> phone_seqs[N_TRANSCRIPTION];
vector<string> filenames;

string changeFileName(string filename) {
	string s = filename.substr(5);
	size_t pos = s.find(".lab\""); // txt파일 찾아서 pos에 저장
	return s.replace(pos, 5, ".txt"); // 형식을 .rec(recognized)으로 바꿔서 반환. 
}

// Read the "trn_mono.txt" file.
void readTranscription() {
	FILE *in = fopen("trn_mono.txt", "r");
	char s[30] = { 0, };
	int tau = 0;
	int phone_index;
	vector<string> tempFilenames;
	vector<string>::iterator tempFilename;

	fseek(in, 8, SEEK_SET);

	while (feof(in) == 0) {
		fscanf(in, "%s", &s);
		if (s[0] == '"') {
			tempFilenames.push_back(s);
			continue;
		}
		if (s[0] == '.') {
			tau++;
			continue;
		}

		phone_index = getPhoneIndex(s);
		phone_seqs[tau].push_back(phone_index);
	}

	// save the inputdata filenames
	for (tempFilename = tempFilenames.begin(); tempFilename != tempFilenames.end(); ++tempFilename) {
		string inputFilename = changeFileName(*tempFilename);
		filenames.push_back(inputFilename);
	}

	fclose(in);
}


void ReadIntputFile() {
	FILE *in = fopen("hmm.txt", "r"); // hmm.txt 읽기

	char str[30];
	int num, n_state, stateIdx, pdfIdx, dimIdx;
	double weightData, meanData, varData, tp;
	

	int p, s, s1, s2, k, d;
	for (p = 0; p < N_PHONE; p++) { // for each N_PHONE
		fseek(in, 3, SEEK_CUR); // ~h pass
		fscanf(in, "%s", &str); // "f"
		phones[p].name = str; // store phone's name
		
		cout << phones[p].name << endl;

		fscanf(in, "%s", &str); // <BEGINHMM>

		fscanf(in, "%s%d", &str, &num); // <NUMSTATES> 5
		n_state = num - 2;
		for (s = 0; s < n_state; s++) {
			fscanf(in, "%s%d", &str, &num); // <STATE> 2
			stateIdx = num - 2;

			fscanf(in, "%s%d", &str, &pdfIdx); // <NUMMIXES> 2
			for (k = 0; k < pdfIdx; k++) {
				fscanf(in, "%s%d%le", &str, &num, &weightData); // <MIXTURE> 1 5.000000e-001
				phones[p].state[stateIdx].pdf[k].weight = weightData; // store weight

				cout << phones[p].state[stateIdx].pdf[k].weight << endl;

				fscanf(in, "%s%d", &str, &dimIdx); // <MEAN> 39
				for (d = 0; d < dimIdx; d++) {
					fscanf(in, "%le", &meanData);
					phones[p].state[stateIdx].pdf[k].mean[d] = meanData; // store means

					cout << phones[p].state[stateIdx].pdf[k].mean[d] << " ";
				}
				cout << endl;

				fscanf(in, "%s%d", &str, &dimIdx); // <VARIANCE> 39
				for (d = 0; d < dimIdx; d++) {
					fscanf(in, "%le", &varData);
					phones[p].state[stateIdx].pdf[k].var[d] = varData; // store variances

					cout << phones[p].state[stateIdx].pdf[k].var[d] << " ";
				}

			}
		}

		fscanf(in, "%s%d", &str, &num); // <TRANSP> 5
		for (s1 = 0; s1 < num; s1++) {
			for (s2 = 0; s2 < num; s2++) {
				fscanf(in, "%le", &tp);
				phones[p].tp[s1][s2] = tp; // store transition probability

				cout << phones[p].tp[s1][s2] << " ";
			}
			cout << endl;
		}

		fscanf(in, "%s", &str); // <ENDHMM>
		fseek(in, 2, SEEK_CUR);
		cout << endl;
		cout << endl;
	}

	fclose(in);
}


void writeOutputFile() {

	FILE *out = fopen("hmm.txt", "w"); // hmm.txt 를 만들고 파일 쓰기

	string tmpStr;
	char tmpDouble[25];

	int p, s, s1, s2, k, d, n_state;
	for (p = 0; p < N_PHONE; p++) {
		fprintf(out, "~h \"");
		fprintf(out, (newPhones[p].name).c_str());
		fprintf(out, "\"\n");
		fprintf(out, "<BEGINHMM>\n");

		fprintf(out, "<NUMSTATES> ");
		if (phones[p].name == "sp") {
			fprintf(out, "3");
		}
		else {
			fprintf(out, "5");
		}

		n_state = getNumberOfPhoneState(p);
		for (s = 0; s < n_state; s++) {
			fprintf(out, "\n");
			fprintf(out, "<STATE> ");
			tmpStr = to_string(s + 2);
			fprintf(out, tmpStr.c_str());

			fprintf(out, "\n");
			fprintf(out, "<NUMMIXES> 2");

			for (k = 0; k < N_PDF; k++) {
				fprintf(out, "\n");
				fprintf(out, "<MIXTURE> ");
				tmpStr = to_string(k + 1);
				fprintf(out, tmpStr.c_str());
				fprintf(out, " ");
				sprintf(tmpDouble, "%le", newPhones[p].state[s].pdf[k].weight);
				fprintf(out, tmpDouble);

				fprintf(out, "\n");
				fprintf(out, "<MEAN> 39\n");
				for (d = 0; d < N_DIMENSION; d++) {
					fprintf(out, " ");
					sprintf(tmpDouble, "%le", newPhones[p].state[s].pdf[k].mean[d]);
					fprintf(out, tmpDouble);
				}

				fprintf(out, "\n");
				fprintf(out, "<VARIANCE> 39\n");
				for (d = 0; d < N_DIMENSION; d++) {
					fprintf(out, " ");
					sprintf(tmpDouble, "%le", newPhones[p].state[s].pdf[k].var[d]);
					fprintf(out, tmpDouble);
				}

			}
		}

		fprintf(out, "\n");
		fprintf(out, "<TRANSP> ");
		tmpStr = to_string(n_state + 2);
		fprintf(out, tmpStr.c_str());
		fprintf(out, "\n");
		for (s1 = 0; s1 < n_state + 2; s1++) {
			for (s2 = 0; s2 < n_state + 2; s2++) {
				sprintf(tmpDouble, "%le", newPhones[p].tp[s1][s2]);
				fprintf(out, tmpDouble);
				fprintf(out, " ");
			}
			fprintf(out, "\n");
		}
		fprintf(out, "<ENDHMM>");
		fprintf(out, "\n");
	}
	

	fclose(out);
}