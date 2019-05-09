#include <math.h>
#include "transition.h"

using namespace std;

vector<transitionType> beginningTransitions; // 시작점에서 들어오는 transition
vector<transitionType> transitions[MAX_PHONE_SEQS][N_STATE]; // phone개수와 state수를 저장
vector<transitionPrevType> transitionsPrev[MAX_PHONE_SEQS][N_STATE];
vector<transitionPrevType> endingTransitions; // 마지막점으로 나가는 transition



// 해당 phone에서 state로 시작할 확률
double getBeginningProb(int phoneIndex, int state) {
	return phones[phoneIndex].tp[0][state + 1]; // trasition probability(tp)행렬 (0, state+1)원소는 해당 state로 시작할 확률(state1, state2, state3이 각각 0, 1, 2으로 인덱싱됨)
}

// 해당 phone state에서 끝날 확률
double getEndingProb(int phoneIndex, int state) {
	int n_state = getNumberOfPhoneState(phoneIndex);  // phone에 있는 state개수를 저장
	return phones[phoneIndex].tp[state + 1][n_state + 1]; // tp행렬 (state + 1, n_state + 1)원소는 해당 state로 끝날 확률
}


// 해당 phone에서 state s -> state d transition 확률
double getTransProb(int phoneIndex, int s, int d) {
	return phones[phoneIndex].tp[s + 1][d + 1]; // tp행렬 (s + 1, d + 1)원소는 s->d로 확률
}

// log를 씌운 후 계산
void applyLogScale(int tau) {
	int i, p, s, p_index, n_state;

	for (i = 0; i < beginningTransitions.size(); i++) {
		beginningTransitions[i].to_prob = log(beginningTransitions[i].to_prob); // 시작점 tp 확률에 log 적용
	}

	for (p = 0; p < phone_seqs[tau].size(); p++) {
		p_index = phone_seqs[tau][p];
		n_state = getNumberOfPhoneState(p_index);
		for (s = 0; s < n_state; s++) {
			for (i = 0; i < transitions[p][s].size(); i++) {
				transitions[p][s][i].to_prob = log(transitions[p][s][i].to_prob); // 다음으로가는 tp 확률에 log 적용
			}
		}
	}

	for (i = 0; i < endingTransitions.size(); i++) {
		endingTransitions[i].from_prob = log(endingTransitions[i].from_prob); // 끝점 tp 확률에 log 적용
	}

	for (p = 0; p < phone_seqs[tau].size(); p++) {
		p_index = phone_seqs[tau][p];
		n_state = getNumberOfPhoneState(p_index);
		for (s = 0; s < n_state; s++) {
			for (i = 0; i < transitionsPrev[p][s].size(); i++) {
				transitionsPrev[p][s][i].from_prob = log(transitionsPrev[p][s][i].from_prob); // 이전으로가는 tp 확률에 log 적용
			}
		}
	}
}

// 모든 transition을 구함
void initTthTransitions(int tau) {
	transitionType t; // transition을 저장할 구조체형 변수 t 선언
	transitionPrevType tPrev;

	// 1. initialize beginning transitions
	//for (int p = 0; p < phone_seqs[tau].size(); p++) {
	int p = 0;
	int p_index = phone_seqs[tau][p];
	int n_state = getNumberOfPhoneState(p_index); // 그 인덱스를 가진 phone의 state 개수 저장
	for (int s = 0; s < n_state; s++) { // 각 state마다
		t.to_phone = p; // phone 인덱스 저장
		t.to_state = s; // state 인덱스 저장
		t.to_prob = getBeginningProb(p_index, s); // 해당 phone의 해당 state에서 v번째(해당 word)의 unigram을 곱하여 tp저장
		if (t.to_prob > 0) beginningTransitions.push_back(t); // 저장한 확률이 0보다 크면, beginningTransitions에 t 저장
	}

		//if (phones[p_index].name == "sp" && p > 0) { // phone이 sp이고 phone index가 0보다 크면
		//	// If the last phone is "sp", then we can skip it
		//	int sp_index = p_index; // sp의 index 저장
		//	double skipProb = phones[sp_index].tp[0][2]; // skip할수있는 transitional probability, 0(들어옴) -> 2(나감)

		//	int next_p_index = p + 1; // sp 이전 phone index 저장
		//	int next_p_n_state = getNumberOfPhoneState(next_p_index); // 그 phone의 state 개수 저장
		//	t.to_phone = p + 1;

		//	for (int next_s = 0; next_s < next_p_n_state; next_s++) { // 각 state 마다
		//		t.to_state = next_s; // 다음 state 저장
		//		t.to_prob = skipProb * getBeginningProb(next_p_index, next_s); // (skip할 확률) * (다음 phone,state에서 확률) 저장
		//		if (t.to_prob > 0) beginningTransitions.push_back(t); // 확률이 0보다 크면 transition에 t 저장
		//	}
		//}
	//}

	// 2. initialize Ending transitions
	//for (int p = 0; p < phone_seqs[tau].size(); p++) {
	p = phone_seqs[tau].size() - 1;
	p_index = phone_seqs[tau][p];
	n_state = getNumberOfPhoneState(p_index); // 그 인덱스를 가진 phone의 state 개수 저장
	for (int s = 0; s < n_state; s++) { // 각 state마다
		tPrev.from_phone = p; // phone 인덱스 저장
		tPrev.from_state = s; // state 인덱스 저장
		tPrev.from_prob = getEndingProb(p_index, s); // 해당 phone의 해당 state에서 v번째(해당 word)의 unigram을 곱하여 tp저장
		if (tPrev.from_prob > 0) endingTransitions.push_back(tPrev); // 저장한 확률이 0보다 크면, beginningTransitions에 t 저장
	}

	//if (phones[p_index].name == "sp" && p_index > 0) { // phone이 sp이고 phone index가 0보다 크면
	//	// If the last phone is "sp", then we can skip it
	//	int sp_index = p_index; // sp의 index 저장
	//	double skipProb = phones[sp_index].tp[0][2]; // skip할수있는 transitional probability, 0(들어옴) -> 2(나감)

	//	int prev_p_index = p - 1; // sp 이전 phone index 저장
	//	int prev_p_n_state = getNumberOfPhoneState(prev_p_index); // 그 phone의 state 개수 저장
	//	tPrev.from_phone = p - 1;

	//	for (int prev_s = 0; prev_s < prev_p_n_state; prev_s++) { // 각 state 마다
	//		tPrev.from_state = prev_s; // 다음 state 저장
	//		tPrev.from_prob = getEndingProb(prev_p_index, prev_s) * skipProb; // (이전 phone, state에서 확률) * (skip할 확률)
	//		if (tPrev.from_prob > 0) endingTransitions.push_back(tPrev); // 확률이 0보다 크면 transition에 t 저장
	//	}
	//}
	//}

	// 3. initialize transitions from each states
	p = 0; // transtion의 phone 인덱스

	// case 0. every transitions of each phone hmm
	vector<int>::iterator phone_seq;
	for (phone_seq = phone_seqs[tau].begin(); phone_seq != phone_seqs[tau].end(); ++phone_seq) {
		int p_index = *phone_seq; // phone index 얻음
		int n_state = getNumberOfPhoneState(p_index); // phone의 state 개수 얻음

		for (int s = 0; s < n_state; s++) { // 각 source state마다
			for (int d = 0; d < n_state; d++) { // 각 destination state 마다
				t.to_phone = p; // phone 인덱스 저장
				t.to_state = d; // dest state 인덱스 저장
				t.to_prob = getTransProb(p_index, s, d); // 해당 phone에서 s state->d state로 갈 확률을 tp에 저장 
				if (t.to_prob > 0) transitions[p][s].push_back(t); // 그 확률이 0보다 크면, transition에 t 저장

				tPrev.from_phone = p;
				tPrev.from_state = s;
				tPrev.from_prob = getTransProb(p_index, s, d);
				if (tPrev.from_prob > 0) transitionsPrev[p][d].push_back(tPrev);
			}
		}
		p++;
	}

	p = 0;
	// case 1. phone to next phone
	for (phone_seq = phone_seqs[tau].begin(); phone_seq != phone_seqs[tau].end() - 1; ++phone_seq) {
		int p_index = *phone_seq; // phone index 저장
		int p_n_state = getNumberOfPhoneState(p_index); // 해당 phone에서 state 개수저장
		int next_p_index = *(phone_seq + 1); // 다음 phone index 저장
		int next_p_n_state = getNumberOfPhoneState(next_p_index); // 다음 phone에서 state 개수저장
		t.to_phone = p + 1; // p + 1 인덱스 저장(다음 phone)
		for (int s = 0; s < p_n_state; s++) { // 각 state 마다
			for (int next_s = 0; next_s < next_p_n_state; next_s++) { // 각 다음 state 마다
				t.to_state = next_s; // 다음 state 저장
				t.to_prob = getEndingProb(p_index, s) * getBeginningProb(next_p_index, next_s); // (이전 phone의 state의 확률 * 다음 phone의 state의 확률)을 저장
				if (t.to_prob > 0) transitions[p][s].push_back(t); // 그 확률이 0보다 크면 transition에 t 저장
			}
		}

		if (phones[p_index].name == "sp" && p > 0) { // voca의 phone이 sp이고 phone index가 0보다 크면
			// If the last phone is "sp", then we can skip it
			int sp_index = p_index; // sp의 index 저장
			double skipProb = phones[sp_index].tp[0][2]; // skip할수있는 transitional probability, 0(들어옴) -> 2(나감)

			int prev_p_index = *(phone_seq - 1); // sp 이전 phone index 저장
			int prev_p_n_state = getNumberOfPhoneState(prev_p_index); // 그 phone의 state 개수 저장

			int next_p_index = *(phone_seq + 1); // 다음 phone index 저장
			int next_p_n_state = getNumberOfPhoneState(next_p_index); // 그 phone의 state 개수 저장
			t.to_phone = p + 1;

			for (int prev_s = 0; prev_s < prev_p_n_state; prev_s++) { // 각 state 마다
				for (int next_s = 0; next_s < next_p_n_state; next_s++) { // 다음 phone의 각 state마다
					t.to_state = next_s; // 다음 state 저장
					t.to_prob = getEndingProb(prev_p_index, prev_s) * skipProb * getBeginningProb(next_p_index, next_s); // (현재 phone, state에서 확률) * (skip할 확률) * (bigram(현재)(다음)) * (다음 phone,state에서 확률) 저장
					if (t.to_prob > 0) transitions[p - 1][prev_s].push_back(t); // 확률이 0보다 크면 transition에 t 저장
				}
			}
		}
		p++;
	}

	p = 1;
	// case 2. phone from prev phone
	for (phone_seq = phone_seqs[tau].begin() + 1; phone_seq != phone_seqs[tau].end(); ++phone_seq) {
		int p_index = *phone_seq; // phone index 저장
		int p_n_state = getNumberOfPhoneState(p_index); // 해당 phone에서 state 개수저장
		int prev_p_index = *(phone_seq - 1); // 이전 phone index 저장
		int prev_p_n_state = getNumberOfPhoneState(prev_p_index); // 이전 phone에서 state 개수저장
		tPrev.from_phone = p - 1; // p - 1 인덱스 저장(이전 phone)
		for (int s = 0; s < p_n_state; s++) { // 각 state 마다
			for (int prev_s = 0; prev_s < prev_p_n_state; prev_s++) { // 각 다음 state 마다
				tPrev.from_state = prev_s; // 다음 state 저장
				tPrev.from_prob = getEndingProb(prev_p_index, prev_s) * getBeginningProb(p_index, s); // (이전 phone의 state의 확률 * 다음 phone의 state의 확률)을 저장
				if (tPrev.from_prob > 0) transitionsPrev[p][s].push_back(tPrev); // 그 확률이 0보다 크면 transition에 t 저장
			}
		}

		if (phones[p_index].name == "sp" && p > 0) { // voca의 phone이 sp이고 phone index가 0보다 크면
			// If the last phone is "sp", then we can skip it
			int sp_index = p_index; // sp의 index 저장
			double skipProb = phones[sp_index].tp[0][2]; // skip할수있는 transitional probability, 0(들어옴) -> 2(나감)

			int next_p_index = *(phone_seq + 1); // 다음 phone index 저장
			int next_p_n_state = getNumberOfPhoneState(next_p_index); // 그 phone의 state 개수 저장

			int prev_p_index = *(phone_seq - 1); // sp 이전 phone index 저장
			int prev_p_n_state = getNumberOfPhoneState(prev_p_index); // 그 phone의 state 개수 저장
			tPrev.from_phone = p - 1;

			for (int next_s = 0; next_s < next_p_n_state; next_s++) { // 각 state 마다
				for (int prev_s = 0; prev_s < prev_p_n_state; prev_s++) { // 다음 phone의 각 state마다
					tPrev.from_state = prev_s;
					tPrev.from_prob = getEndingProb(prev_p_index, prev_s) * skipProb * getBeginningProb(next_p_index, next_s); // (현재 phone, state에서 확률) * (skip할 확률) * (bigram(현재)(다음)) * (다음 phone,state에서 확률) 저장
					if (tPrev.from_prob > 0) transitionsPrev[p + 1][next_s].push_back(tPrev); // 확률이 0보다 크면 transition에 t 저장
				}
			}
		}
		p++;
	}

	applyLogScale(tau); // log scale로 확률 변환

}
