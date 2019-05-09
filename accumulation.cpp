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
		n_state = getNumberOfPhoneState(p); // 'sp'�� state���� 1, �������� 3
		for (s = 0; s < n_state; s++) {
			observationProb[p][s] = getObservationProb(p, s, spectrum); // �� phone���� �� state������ b_s(x)Ȯ������ ���� 
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
				alphas[t][p][s].isAssigned = false; // �� voxel�� false�Ҵ�
				alphas[t][p][s].prob = 0;
			}
		}
	}
}

void runForward(int length, double spectrogram[][N_DIMENSION], int tau) {
	int t, p, s; // time, phone, state �ε���
	vector<transitionPrevType>::iterator transPrev;
	valueType *alpha; // ���İ�����
	vector<double> alphaVecs;
	vector<double>::iterator alphaVec;
	double currentProb; // ����Ȯ��
	double sum, max; // ����� ���� �ӽú���
	int p_index, n_state; // ���� phone �ε���

	resetAlphas(length, tau); // value������ isassigned �� false�� �ʱ�ȭ
	
	
	memoizeObservationProbs(spectrogram[0]);
	alpha = &alphas[0][0][0]; // ù��° alpha��.
	p_index = phone_seqs[tau][0]; // 0~20���� phone index�� ��ȯ
	currentProb = beginningTransitions.begin()->to_prob + observationProb[p_index][0]; // currentprob�� transition prob�� observation prob�� ����(log scale�̹Ƿ� ����)
	if (!alpha->isAssigned) { // value�� �Ҵ���� �ʾ�����
		alpha->isAssigned = true; // value�� �Ҵ�Ǿ��ٰ� ǥ��
		alpha->prob = currentProb; // value->prob �� ����Ȯ���� �Ҵ�

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

				if (!alpha->isAssigned) { // value�� �Ҵ���� �ʾ�����
					alpha->isAssigned = true; // value�� true�� assign
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
				betas[t][p][s].isAssigned = false; // �� voxel�� false�Ҵ�
				betas[t][p][s].prob = 0;
			}
		}
	}
}

void runBackward(int length, double spectrogram[][N_DIMENSION], int tau) {
	int t, p, s; // time, phone, state �ε���
	vector<transitionPrevType>::iterator trans; // transition ������ ����
	vector<transitionType>::iterator transNext;
	valueType *beta; // ���İ�����
	vector<double> betaVecs;
	vector<double>::iterator betaVec;
	double currentProb; // ����Ȯ��
	double sum; // ����� ���� �ӽú���
	double max; // ����� ���� �ӽú���
	int p_index, n_state, next_p_index;

	resetBetas(length, tau); // value������ isassigned �� false�� �ʱ�ȭ

	for (trans = endingTransitions.begin(); trans != endingTransitions.end(); ++trans) {
		beta = &betas[length - 1][trans->from_phone][trans->from_state]; // ù��° alpha��.
		currentProb = trans->from_prob;
		if (!beta->isAssigned) { // value�� �Ҵ���� �ʾ�����
			beta->isAssigned = true; // value�� �Ҵ�Ǿ��ٰ� ǥ��
			beta->prob = currentProb; // value->prob �� ����Ȯ���� �Ҵ�

			//cout << trans->to_phone << " " << trans->to_state << " alpha0," << trans->to_phone * 3 + trans->to_state<< " = " << alpha->prob << endl;
		}
	}

	for (t = length - 2; t >= 0; t--) { // �� time length�� ���Ͽ�
		memoizeObservationProbs(spectrogram[t + 1]); // t time�� ���Ͽ� observation prob�� ����
		for (p = 0; p < phone_seqs[tau].size(); p++) {
			p_index = phone_seqs[tau][p]; // p��° phone �ε��� ����
			n_state = getNumberOfPhoneState(p_index); // ������ ������ �ε������� state���� ����
			for (s = 0; s < n_state; s++) { // �� state�� ���Ͽ�
				beta = &betas[t][p][s]; // ���� time(t+1)�� ���Ͽ� value �Ҵ�

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

				if (!beta->isAssigned) { // value�� �Ҵ���� �ʾ�����
					beta->isAssigned = true; // value�� true�� assign
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

	int t, p, s, d, next_p, next_s, k; // �� index
	int p_index, n_state, next_p_index;
	vector<transitionType>::iterator transit;
	double gamma_it, ksi_ijt, gamma_ikt;
	double *nextObserProb[N_PHONE];

	int count = 0;
	for (t = 0; t < length; t++) { //length
		
		double probSum = 0;
		count++;

		totalProbForGamma = getTotalProbForGamma(t, tau);

		memoizeObservationProbs(spectrogram[t + 1]); // t+1 time�� ���Ͽ� observation prob�� ����
		for (int p = 0; p < N_PHONE; p++) {
			nextObserProb[p] = &observationProb[p][0];
		}
		
		/*if (t != length - 1) {
			totalProbForKsi = getTotalProbForKsi(t, tau);
		}*/

		memoizeObservationProbs(spectrogram[t]); // t time�� ���Ͽ� observation prob�� ����
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
					// ��� time, trascription�� ���Ͽ� i��° gamma �� -> �и� ���
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

	// vector �ʱ�ȭ
	beginningTransitions.clear();
	for (int p = 0; p < phone_seqs[tau].size(); p++) {
		int p_index = phone_seqs[tau][p]; // p��° phone �ε��� ����
		int n_state = getNumberOfPhoneState(p_index); // ������ ������ �ε������� state���� ����
		for (int s = 0; s < n_state; s++) { // �� state�� ���Ͽ�
			transitions[p][s].clear();
		}
	}
	endingTransitions.clear();
	for (int p = 0; p < phone_seqs[tau].size(); p++) {
		int p_index = phone_seqs[tau][p]; // p��° phone �ε��� ����
		int n_state = getNumberOfPhoneState(p_index); // ������ ������ �ε������� state���� ����
		for (int s = 0; s < n_state; s++) { // �� state�� ���Ͽ�
			transitionsPrev[p][s].clear();
		}
	}
	
}