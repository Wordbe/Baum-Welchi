#define _CRT_SECURE_NO_WARNINGS

#include "baumwelch.h"
#include "fileutil.h"
#include <iostream>

double firstGamma[N_PHONE][N_STATE];
double lastGamma[N_PHONE][N_STATE];
double sumGamma[N_PHONE][N_STATE];
double sumGammaForKsi[N_PHONE][N_STATE];
double ksi[N_PHONE][N_STATE][N_STATE];
double sumGammaK[N_PHONE][N_STATE][N_PDF];
double means[N_PHONE][N_STATE][N_PDF][N_DIMENSION];
double vars[N_PHONE][N_STATE][N_PDF][N_DIMENSION];

phoneType newPhones[N_PHONE];

void resetNewPhones() {
	int p, i, j, n_state;
	for (p = 0; p < N_PHONE; p++) {
		n_state = getNumberOfPhoneState(p);
		for (i = 0; i < n_state + 2; i++) {
			for (j = 0; j < n_state + 2; j++) {
				newPhones[p].tp[i][j] = 0.0;
			}
		}
	}
}
void resetFirstGamma() {
	int p, s, n_state;
	for (p = 0; p < N_PHONE; p++) {
		n_state = getNumberOfPhoneState(p);
		for (s = 0; s < n_state; s++) {
			firstGamma[p][s] = 0.0;
		}
	}
}
void resetLastGamma() {
	int p, s, n_state;
	for (p = 0; p < N_PHONE; p++) {
		n_state = getNumberOfPhoneState(p);
		for (s = 0; s < n_state; s++) {
			lastGamma[p][s] = 0.0;
		}
	}
}
void resetSumGamma() {
	int p, s, n_state;
	for (p = 0; p < N_PHONE; p++) {
		n_state = getNumberOfPhoneState(p);
		for (s = 0; s < n_state; s++) {
			sumGamma[p][s] = 0.0;
		}
	}
}
void resetSumGammaForKsi() {
	int p, s, n_state;
	for (p = 0; p < N_PHONE; p++) {
		n_state = getNumberOfPhoneState(p);
		for (s = 0; s < n_state; s++) {
			sumGammaForKsi[p][s] = 0.0;
		}
	}
}
void resetKsi() {
	int p, s, next_s, n_state;
	for (p = 0; p < N_PHONE; p++) {
		n_state = getNumberOfPhoneState(p);
		for (s = 0; s < n_state; s++) {
			for (next_s = 0; next_s < n_state; next_s++) {
				ksi[p][s][next_s] = 0.0;
			}
		}
	}
}
void resetSumGammaK() {
	int p, s, k, n_state;
	for (p = 0; p < N_PHONE; p++) {
		n_state = getNumberOfPhoneState(p);
		for (s = 0; s < n_state; s++) {
			for (k = 0; k < N_PDF; k++) {
				sumGammaK[p][s][k] = 0.0;
			}
		}
	}
}
void resetMeans() {
	int p, s, k, d, n_state;
	for (p = 0; p < N_PHONE; p++) {
		n_state = getNumberOfPhoneState(p);
		for (s = 0; s < n_state; s++) {
			for (k = 0; k < N_PDF; k++) {
				for (d = 0; d < N_DIMENSION; d++) {
					means[p][s][k][d] = 0.0;
				}
			}
		}
	}
}
void resetVars() {
	int p, s, k, d, n_state;
	for (p = 0; p < N_PHONE; p++) {
		n_state = getNumberOfPhoneState(p);
		for (s = 0; s < n_state; s++) {
			for (k = 0; k < N_PDF; k++) {
				for (d = 0; d < N_DIMENSION; d++) {
					vars[p][s][k][d] = 0.0;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------------------------------
int length, dimension; // 길이수 변수 선언
double spectrogram[MAX_TIME_LENGTH][N_DIMENSION]; // 인풋 x (spectrogram)선언

// Read training input data
void readTestInput(string path) {
	FILE *in = fopen(path.c_str(), "r"); // path에 있는 파일 open, read 모드
	int t, d; // 길이인덱스 t, 차원인덱스 d, 차원수 dimension 변수 선언

	fscanf(in, "%d%d", &length, &dimension); // 파일데이터 첫줄에서 길이와 차원을 각각 length, dimension에 저장

	for (t = 0; t < length; t++) {
		for (d = 0; d < dimension; d++) {
			fscanf(in, "%lf", &spectrogram[t][d]); // spectrogram에 인풋데이터 저장
		}
	}

	fclose(in);
}

// --------------------- Run the Baum-Welch Algorithm ---------------------------------------------------------------------------------------

void runBaumWelch() {
	
	resetNewPhones();
	resetFirstGamma();
	resetLastGamma();
	resetSumGamma();
	resetSumGammaForKsi();
	resetKsi();
	resetSumGammaK();
	resetMeans();
	resetVars();


	// E - Step
	int count = 0;
	for (int tau = 0; tau < N_TRANSCRIPTION; tau++) { // for each N_TRANSCRIPTION
		initTthTransitions(tau); // 모든 transition을 만듦 (파라미터 만듦, A, B)

		string inputFile = filenames[tau];
		cout << inputFile << endl;
		readTestInput(inputFile); // assign value for length, dimension, spectrogram
		
		runAcculmulation(length, spectrogram, tau);

		count++;
		if (count % 20 == 0) printf("%.2lf%%..\n", (double)count / (double)N_TRANSCRIPTION * 100);
	}

	// M - Step
	int p, s, s1, next_s, s2, s3, k, d;
	int n_state;
	double mean;

	for (p = 0; p < N_PHONE; p++) {
		newPhones[p].name = phones[p].name;
		n_state = getNumberOfPhoneState(p);
		for (s = 0; s < n_state; s++) {
			newPhones[p].tp[0][s + 1] = firstGamma[p][s] / N_TRANSCRIPTION; //  a_0j
		}
		
		for (s1 = 0; s1 < n_state; s1++) {
			for (next_s = 0; next_s < n_state; next_s++) {
				newPhones[p].tp[s1 + 1][next_s + 1] = ksi[p][s1][next_s] / sumGamma[p][s1]; // a_ij

				cout << p << "," << s1 + 1 << "," << next_s + 1 << " " << ksi[p][s1][next_s] << " " << sumGamma[p][s1] << " " << newPhones[p].tp[s1 + 1][next_s + 1] << endl;
			}
		}

		for (s2 = 0; s2 < n_state; s2++) {
			newPhones[p].tp[s2 + 1][4] = lastGamma[p][s2] / sumGamma[p][s2]; // a_i,N+1
		}

		for (s3 = 0; s3 < n_state; s3++) {
			for (k = 0; k < N_PDF; k++) {
				newPhones[p].state[s3].pdf[k].weight = sumGammaK[p][s3][k] / sumGamma[p][s3]; // c_ik

				for (d = 0; d < N_DIMENSION; d++) {
					mean = means[p][s3][k][d] / sumGammaK[p][s3][k];
					newPhones[p].state[s3].pdf[k].mean[d] = mean; // mean_ik
					newPhones[p].state[s3].pdf[k].var[d] = vars[p][s3][k][d] / sumGammaK[p][s3][k] - mean * mean; // var_ik

				}
			}
		}
	}

}