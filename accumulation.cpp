#include "accumulation.h"
#include "baumwelch.h"
#include <iostream>

using namespace std;

// --------------- 1. get observation probability ---------------

double observationProb[N_PHONE][N_STATE];

void memoizeObservationProbs(double spectrum[]) {
	int p, s;
	int n_state;
	for (p = 0; p < N_PHONE; p++) {
		n_state = getNumberOfPhoneState(p); // 'sp'는 state개수 1, 나머지는 3
		for (s = 0; s < n_state; s++) {
			observationProb[p][s] = getObservationProb(p, s, spectrum); // 각 phone마다 각 state에서의 b_s(x)확률값을 저장 
		}
	}
}



// --------------- 2. get alpha ------------------------------------

valueType alphas[MAX_TIME_LENGTH][MAX_PHONE_SEQS][N_STATE];

void resetAlphas(int length, int tau) {
	int t, p, s;
	for (t = 0; t < length; t++) {
		for (p = 0; p < phone_seqs[tau].size(); p++) {
			for (s = 0; s < N_STATE; s++) {
				alphas[t][p][s].isAssigned = false; // 각 voxel에 false할당
				alphas[t][p][s].prob = 0;
			}
		}
	}
}

void runForward(int length, double spectrogram[][N_DIMENSION], int tau) {
	int t, p, s; // time, phone, state 인덱스
	vector<transitionPrevType>::iterator transPrev;
	valueType *alpha; // 알파값저장
	vector<double> alphaVecs;
	vector<double>::iterator alphaVec;
	double currentProb; // 현재확률
	double sum, max; // 계산을 위한 임시변수
	int p_index, n_state; // 다음 phone 인덱스

	resetAlphas(length, tau); // value변수의 isassigned 값 false로 초기화
	
	
	memoizeObservationProbs(spectrogram[0]);
	alpha = &alphas[0][0][0]; // 첫번째 alpha값.
	p_index = phone_seqs[tau][0]; // 0~20사이 phone index로 전환
	currentProb = beginningTransitions.begin()->to_prob + observationProb[p_index][0]; // currentprob에 transition prob과 observation prob을 곱합(log scale이므로 더함)
	if (!alpha->isAssigned) { // value가 할당되지 않았으면
		alpha->isAssigned = true; // value가 할당되었다고 표시
		alpha->prob = currentProb; // value->prob 에 현재확률을 할당

		//cout << trans->to_phone << " " << trans->to_state << " alpha0," << trans->to_phone * 3 + trans->to_state<< " = " << alpha->prob << endl;
	}

	for (t = 1; t < length; t++) {
		memoizeObservationProbs(spectrogram[t]);
		for (p = 0; p < phone_seqs[tau].size(); p++) {
			p_index = phone_seqs[tau][p];
			n_state = getNumberOfPhoneState(p_index);
			for (s = 0; s < n_state; s++) {
				alpha = &alphas[t][p][s];

				for (transPrev = transitionsPrev[p][s].begin(); transPrev != transitionsPrev[p][s].end(); ++transPrev) {
					alphaVecs.push_back(alphas[t - 1][transPrev->from_phone][transPrev->from_state].prob + transPrev->from_prob);
				}

				if (alphaVecs.empty()) continue;
				max = *alphaVecs.begin();
				sum = 0;
				for (alphaVec = alphaVecs.begin(); alphaVec != alphaVecs.end(); ++alphaVec) {
					if (max < *alphaVec) max = *alphaVec;
				}
				for (alphaVec = alphaVecs.begin(); alphaVec != alphaVecs.end(); ++alphaVec) {
					sum += exp(*alphaVec - max);
				}

				currentProb = max + log(sum) + observationProb[p_index][s];

				if (!alpha->isAssigned) { // value가 할당되지 않았으면
					alpha->isAssigned = true; // value를 true로 assign
					alpha->prob = currentProb;
				}

				alphaVecs.clear();
			}
		}
	}
}




// --------------- 3. get beta -------------------------------------

valueType betas[MAX_TIME_LENGTH][MAX_PHONE_SEQS][N_STATE];

void resetBetas(int length, int tau) {
	int t, p, s;
	for (t = 0; t < length; t++) {
		for (p = 0; p < phone_seqs[tau].size(); p++) {
			for (s = 0; s < N_STATE; s++) {
				betas[t][p][s].isAssigned = false; // 각 voxel에 false할당
				betas[t][p][s].prob = 0;
			}
		}
	}
}

void runBackward(int length, double spectrogram[][N_DIMENSION], int tau) {
	int t, p, s; // time, phone, state 인덱스
	vector<transitionPrevType>::iterator trans; // transition 저장할 변수
	vector<transitionType>::iterator transNext;
	valueType *beta; // 알파값저장
	vector<double> betaVecs;
	vector<double>::iterator betaVec;
	double currentProb; // 현재확률
	double sum; // 계산을 위한 임시변수
	double max; // 계산을 위한 임시변수
	int p_index, n_state, next_p_index;

	resetBetas(length, tau); // value변수의 isassigned 값 false로 초기화

	for (trans = endingTransitions.begin(); trans != endingTransitions.end(); ++trans) {
		beta = &betas[length - 1][trans->from_phone][trans->from_state]; // 첫번째 alpha값.
		currentProb = trans->from_prob;
		if (!beta->isAssigned) { // value가 할당되지 않았으면
			beta->isAssigned = true; // value가 할당되었다고 표시
			beta->prob = currentProb; // value->prob 에 현재확률을 할당

			//cout << trans->to_phone << " " << trans->to_state << " alpha0," << trans->to_phone * 3 + trans->to_state<< " = " << alpha->prob << endl;
		}
	}

	for (t = length - 2; t >= 0; t--) { // 각 time length에 대하여
		memoizeObservationProbs(spectrogram[t + 1]); // t time에 대하여 observation prob을 저장
		for (p = 0; p < phone_seqs[tau].size(); p++) {
			p_index = phone_seqs[tau][p]; // p번째 phone 인덱스 저장
			n_state = getNumberOfPhoneState(p_index); // 위에서 저장한 인덱스에서 state개수 저장
			for (s = 0; s < n_state; s++) { // 각 state에 대하여
				beta = &betas[t][p][s]; // 이전 time(t+1)에 대하여 value 할당

				for (transNext = transitions[p][s].begin(); transNext != transitions[p][s].end(); ++transNext) {
					next_p_index = phone_seqs[tau][transNext->to_phone];
					betaVecs.push_back(betas[t + 1][transNext->to_phone][transNext->to_state].prob + transNext->to_prob + observationProb[next_p_index][transNext->to_state]);
				}

				if (betaVecs.empty()) continue;
				max = *betaVecs.begin();
				sum = 0;
				for (betaVec = betaVecs.begin(); betaVec != betaVecs.end(); ++betaVec) {
					if (max < *betaVec) max = *betaVec;
				}
				for (betaVec = betaVecs.begin(); betaVec != betaVecs.end(); ++betaVec) {
					sum += exp(*betaVec - max);
				}

				currentProb = max + log(sum);

				if (!beta->isAssigned) { // value가 할당되지 않았으면
					beta->isAssigned = true; // value를 true로 assign
					beta->prob = currentProb;
				}

				betaVecs.clear();
			}
		}
	}

	/*for (t = 0; t < 1; t++) {
		for (p = 0; p < phone_seqs[tau].size(); p++) {
			int p_index = phone_seqs[tau][p];
			int n_state = getNumberOfPhoneState(p_index);
			for (s = 0; s < n_state; s++) {
				cout << betas[t][p][s].prob << " ";
			}
			cout << endl;
		}
	}*/
}


// --------------- 4. get total probabilty ------------------------------

// sum_i(alpha_i * beta_i)
double getTotalProbForGamma(int t, int tau) {
	vector<double> probs;
	vector<double>::iterator prob;
	int p, s, p_index, n_state;
	double max, sum;
	for (p = 0; p < phone_seqs[tau].size(); p++) {
		p_index = phone_seqs[tau][p];
		n_state = getNumberOfPhoneState(p_index);
		for (s = 0; s < n_state; s++) {
			if (alphas[t][p][s].isAssigned && betas[t][p][s].isAssigned) {
				probs.push_back(alphas[t][p][s].prob + betas[t][p][s].prob);
			}
		}
	}
	max = *probs.begin();
	sum = 0;
	for (prob = probs.begin(); prob != probs.end(); ++prob) {
		if (max < *prob) max = *prob;
	}
	for (prob = probs.begin(); prob != probs.end(); ++prob) {
		sum += exp(*prob - max);
	}

	probs.clear();

	return max + log(sum);
}

// --------------- 5. Run Acculumation -------------------------------------------

void runAcculmulation(int length, double spectrogram[][N_DIMENSION], int tau) {
	runForward(length, spectrogram, tau);
	runBackward(length, spectrogram, tau);
	double totalProbForGamma;
	//double totalProbForKsi;

	int t, p, s, d, next_p, next_s, k; // 각 index
	int p_index, n_state, next_p_index;
	vector<transitionType>::iterator transit;
	double gamma_it, ksi_ijt, gamma_ikt;
	double *nextObserProb[N_PHONE];

	int count = 0;
	for (t = 0; t < length; t++) { //length
		
		double probSum = 0;
		count++;

		totalProbForGamma = getTotalProbForGamma(t, tau);

		memoizeObservationProbs(spectrogram[t + 1]); // t+1 time에 대하여 observation prob을 저장
		for (int p = 0; p < N_PHONE; p++) {
			nextObserProb[p] = &observationProb[p][0];
		}
		
		/*if (t != length - 1) {
			totalProbForKsi = getTotalProbForKsi(t, tau);
		}*/

		memoizeObservationProbs(spectrogram[t]); // t time에 대하여 observation prob을 저장
		for (p = 0; p < phone_seqs[tau].size(); p++) {
			p_index = phone_seqs[tau][p];
			n_state = getNumberOfPhoneState(p_index);
			for (s = 0; s < n_state; s++) {
				if (alphas[t][p][s].isAssigned && betas[t][p][s].isAssigned) {
					gamma_it = exp(alphas[t][p][s].prob + betas[t][p][s].prob - totalProbForGamma);

					cout << gamma_it << endl;
					probSum += gamma_it;
					/*cout << endl;
					cout << betas[t][p][s].prob << endl;*/
					//cout << t << "," << p << "," << s << " " << alphas[t][p][s].prob + betas[t][p][s].prob << " " << totalProbForGamma << endl;
					//cout << t << "," << p << "," << s << " " << gamma_it << endl;
					// 모든 time, trascription에 대하여 i번째 gamma 합 -> 분모에 사용
					sumGamma[p_index][s] += gamma_it;

					if (t == 0) {
						firstGamma[p_index][s] += gamma_it;
					}

					
					//sumGammaForKsi[p_index][s] += gamma_it;

					if (t != length - 1) { 
						for (transit = transitions[p][s].begin(); transit != transitions[p][s].end(); ++transit) {
							next_p = transit->to_phone;
							next_s = transit->to_state;
							next_p_index = phone_seqs[tau][next_p];
							if (betas[t + 1][next_p][next_s].isAssigned) {
								ksi_ijt = exp(alphas[t][p][s].prob + transit->to_prob + nextObserProb[next_p_index][next_s] + betas[t + 1][next_p][next_s].prob - totalProbForGamma);
								//cout << transit->to_prob + nextObserProb[next_p_index][next_s] + betas[t + 1][next_p][next_s].prob << " ";
								if (p == next_p) {
									
									
									ksi[p_index][s][next_s] += ksi_ijt;


									
									/*cout << "time= " << t << " " << p << " " << s << " " << "->" << next_p << " " << next_s << " " << ksi_ijt << " " <<
										alphas[t][p][s].prob << " " << transit->to_prob << " " << nextObserProb[next_p_index][next_s] << " " << betas[t + 1][next_p][next_s].prob << " "
										<< alphas[t][p][s].prob + transit->to_prob + nextObserProb[next_p_index][next_s] + betas[t + 1][next_p][next_s].prob << " " << totalProbForKsi << " " <<
										alphas[t][p][s].prob + transit->to_prob + nextObserProb[next_p_index][next_s] + betas[t + 1][next_p][next_s].prob - totalProbForKsi << endl;*/

										/*if (t == 2) {
											cout << "time= " << t << " " << p << " " << s << " " << "->" << next_p << " " << next_s << " " << ksi_ijt << " " <<
											alphas[t][p][s].prob << " " << transit->to_prob << " " << nextObserProb[next_p_index][next_s] << " " << betas[t + 1][next_p][next_s].prob << " "
											<< alphas[t][p][s].prob + transit->to_prob + nextObserProb[next_p_index][next_s] + betas[t + 1][next_p][next_s].prob << " " << totalProbForKsi << " " <<
											alphas[t][p][s].prob + transit->to_prob + nextObserProb[next_p_index][next_s] + betas[t + 1][next_p][next_s].prob - totalProbForKsi << endl;
										}*/
								}
								else if (p + 1 == next_p) {
									lastGamma[p_index][s] += ksi_ijt;
									//sumGammaForKsi[p_index][s] += gamma_it;
								
								}
							}
						}
					}

					/*if (t == length - 1) {
						lastGamma[p_index][s] += gamma_it;
					}*/

					// for each Gaussian k
					for (k = 0; k < N_PDF; k++) {
						gamma_ikt = gamma_it * exp(log(phones[p_index].state[s].pdf[k].weight) + probForOnePDF(p_index, s, k, spectrogram[t]) - observationProb[p_index][s]);

						sumGammaK[p_index][s][k] += gamma_ikt;

						for (d = 0; d < N_DIMENSION; d++) {
							means[p_index][s][k][d] += gamma_ikt * spectrogram[t][d];
							vars[p_index][s][k][d] += gamma_ikt * spectrogram[t][d] * spectrogram[t][d];
						}
					}
				}
			}
		}
		cout << "count " << count << " " << probSum;
		cout << endl;
	}

	// vector 초기화
	beginningTransitions.clear();
	for (int p = 0; p < phone_seqs[tau].size(); p++) {
		int p_index = phone_seqs[tau][p]; // p번째 phone 인덱스 저장
		int n_state = getNumberOfPhoneState(p_index); // 위에서 저장한 인덱스에서 state개수 저장
		for (int s = 0; s < n_state; s++) { // 각 state에 대하여
			transitions[p][s].clear();
		}
	}
	endingTransitions.clear();
	for (int p = 0; p < phone_seqs[tau].size(); p++) {
		int p_index = phone_seqs[tau][p]; // p번째 phone 인덱스 저장
		int n_state = getNumberOfPhoneState(p_index); // 위에서 저장한 인덱스에서 state개수 저장
		for (int s = 0; s < n_state; s++) { // 각 state에 대하여
			transitionsPrev[p][s].clear();
		}
	}
	
}