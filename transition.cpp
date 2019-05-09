#include <math.h>
#include "transition.h"

using namespace std;

vector<transitionType> beginningTransitions; // ���������� ������ transition
vector<transitionType> transitions[MAX_PHONE_SEQS][N_STATE]; // phone������ state���� ����
vector<transitionPrevType> transitionsPrev[MAX_PHONE_SEQS][N_STATE];
vector<transitionPrevType> endingTransitions; // ������������ ������ transition



// �ش� phone���� state�� ������ Ȯ��
double getBeginningProb(int phoneIndex, int state) {
	return phones[phoneIndex].tp[0][state + 1]; // trasition probability(tp)��� (0, state+1)���Ҵ� �ش� state�� ������ Ȯ��(state1, state2, state3�� ���� 0, 1, 2���� �ε��̵�)
}

// �ش� phone state���� ���� Ȯ��
double getEndingProb(int phoneIndex, int state) {
	int n_state = getNumberOfPhoneState(phoneIndex);  // phone�� �ִ� state������ ����
	return phones[phoneIndex].tp[state + 1][n_state + 1]; // tp��� (state + 1, n_state + 1)���Ҵ� �ش� state�� ���� Ȯ��
}


// �ش� phone���� state s -> state d transition Ȯ��
double getTransProb(int phoneIndex, int s, int d) {
	return phones[phoneIndex].tp[s + 1][d + 1]; // tp��� (s + 1, d + 1)���Ҵ� s->d�� Ȯ��
}

// log�� ���� �� ���
void applyLogScale(int tau) {
	int i, p, s, p_index, n_state;

	for (i = 0; i < beginningTransitions.size(); i++) {
		beginningTransitions[i].to_prob = log(beginningTransitions[i].to_prob); // ������ tp Ȯ���� log ����
	}

	for (p = 0; p < phone_seqs[tau].size(); p++) {
		p_index = phone_seqs[tau][p];
		n_state = getNumberOfPhoneState(p_index);
		for (s = 0; s < n_state; s++) {
			for (i = 0; i < transitions[p][s].size(); i++) {
				transitions[p][s][i].to_prob = log(transitions[p][s][i].to_prob); // �������ΰ��� tp Ȯ���� log ����
			}
		}
	}

	for (i = 0; i < endingTransitions.size(); i++) {
		endingTransitions[i].from_prob = log(endingTransitions[i].from_prob); // ���� tp Ȯ���� log ����
	}

	for (p = 0; p < phone_seqs[tau].size(); p++) {
		p_index = phone_seqs[tau][p];
		n_state = getNumberOfPhoneState(p_index);
		for (s = 0; s < n_state; s++) {
			for (i = 0; i < transitionsPrev[p][s].size(); i++) {
				transitionsPrev[p][s][i].from_prob = log(transitionsPrev[p][s][i].from_prob); // �������ΰ��� tp Ȯ���� log ����
			}
		}
	}
}

// ��� transition�� ����
void initTthTransitions(int tau) {
	transitionType t; // transition�� ������ ����ü�� ���� t ����
	transitionPrevType tPrev;

	// 1. initialize beginning transitions
	//for (int p = 0; p < phone_seqs[tau].size(); p++) {
	int p = 0;
	int p_index = phone_seqs[tau][p];
	int n_state = getNumberOfPhoneState(p_index); // �� �ε����� ���� phone�� state ���� ����
	for (int s = 0; s < n_state; s++) { // �� state����
		t.to_phone = p; // phone �ε��� ����
		t.to_state = s; // state �ε��� ����
		t.to_prob = getBeginningProb(p_index, s); // �ش� phone�� �ش� state���� v��°(�ش� word)�� unigram�� ���Ͽ� tp����
		if (t.to_prob > 0) beginningTransitions.push_back(t); // ������ Ȯ���� 0���� ũ��, beginningTransitions�� t ����
	}

		//if (phones[p_index].name == "sp" && p > 0) { // phone�� sp�̰� phone index�� 0���� ũ��
		//	// If the last phone is "sp", then we can skip it
		//	int sp_index = p_index; // sp�� index ����
		//	double skipProb = phones[sp_index].tp[0][2]; // skip�Ҽ��ִ� transitional probability, 0(����) -> 2(����)

		//	int next_p_index = p + 1; // sp ���� phone index ����
		//	int next_p_n_state = getNumberOfPhoneState(next_p_index); // �� phone�� state ���� ����
		//	t.to_phone = p + 1;

		//	for (int next_s = 0; next_s < next_p_n_state; next_s++) { // �� state ����
		//		t.to_state = next_s; // ���� state ����
		//		t.to_prob = skipProb * getBeginningProb(next_p_index, next_s); // (skip�� Ȯ��) * (���� phone,state���� Ȯ��) ����
		//		if (t.to_prob > 0) beginningTransitions.push_back(t); // Ȯ���� 0���� ũ�� transition�� t ����
		//	}
		//}
	//}

	// 2. initialize Ending transitions
	//for (int p = 0; p < phone_seqs[tau].size(); p++) {
	p = phone_seqs[tau].size() - 1;
	p_index = phone_seqs[tau][p];
	n_state = getNumberOfPhoneState(p_index); // �� �ε����� ���� phone�� state ���� ����
	for (int s = 0; s < n_state; s++) { // �� state����
		tPrev.from_phone = p; // phone �ε��� ����
		tPrev.from_state = s; // state �ε��� ����
		tPrev.from_prob = getEndingProb(p_index, s); // �ش� phone�� �ش� state���� v��°(�ش� word)�� unigram�� ���Ͽ� tp����
		if (tPrev.from_prob > 0) endingTransitions.push_back(tPrev); // ������ Ȯ���� 0���� ũ��, beginningTransitions�� t ����
	}

	//if (phones[p_index].name == "sp" && p_index > 0) { // phone�� sp�̰� phone index�� 0���� ũ��
	//	// If the last phone is "sp", then we can skip it
	//	int sp_index = p_index; // sp�� index ����
	//	double skipProb = phones[sp_index].tp[0][2]; // skip�Ҽ��ִ� transitional probability, 0(����) -> 2(����)

	//	int prev_p_index = p - 1; // sp ���� phone index ����
	//	int prev_p_n_state = getNumberOfPhoneState(prev_p_index); // �� phone�� state ���� ����
	//	tPrev.from_phone = p - 1;

	//	for (int prev_s = 0; prev_s < prev_p_n_state; prev_s++) { // �� state ����
	//		tPrev.from_state = prev_s; // ���� state ����
	//		tPrev.from_prob = getEndingProb(prev_p_index, prev_s) * skipProb; // (���� phone, state���� Ȯ��) * (skip�� Ȯ��)
	//		if (tPrev.from_prob > 0) endingTransitions.push_back(tPrev); // Ȯ���� 0���� ũ�� transition�� t ����
	//	}
	//}
	//}

	// 3. initialize transitions from each states
	p = 0; // transtion�� phone �ε���

	// case 0. every transitions of each phone hmm
	vector<int>::iterator phone_seq;
	for (phone_seq = phone_seqs[tau].begin(); phone_seq != phone_seqs[tau].end(); ++phone_seq) {
		int p_index = *phone_seq; // phone index ����
		int n_state = getNumberOfPhoneState(p_index); // phone�� state ���� ����

		for (int s = 0; s < n_state; s++) { // �� source state����
			for (int d = 0; d < n_state; d++) { // �� destination state ����
				t.to_phone = p; // phone �ε��� ����
				t.to_state = d; // dest state �ε��� ����
				t.to_prob = getTransProb(p_index, s, d); // �ش� phone���� s state->d state�� �� Ȯ���� tp�� ���� 
				if (t.to_prob > 0) transitions[p][s].push_back(t); // �� Ȯ���� 0���� ũ��, transition�� t ����

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
		int p_index = *phone_seq; // phone index ����
		int p_n_state = getNumberOfPhoneState(p_index); // �ش� phone���� state ��������
		int next_p_index = *(phone_seq + 1); // ���� phone index ����
		int next_p_n_state = getNumberOfPhoneState(next_p_index); // ���� phone���� state ��������
		t.to_phone = p + 1; // p + 1 �ε��� ����(���� phone)
		for (int s = 0; s < p_n_state; s++) { // �� state ����
			for (int next_s = 0; next_s < next_p_n_state; next_s++) { // �� ���� state ����
				t.to_state = next_s; // ���� state ����
				t.to_prob = getEndingProb(p_index, s) * getBeginningProb(next_p_index, next_s); // (���� phone�� state�� Ȯ�� * ���� phone�� state�� Ȯ��)�� ����
				if (t.to_prob > 0) transitions[p][s].push_back(t); // �� Ȯ���� 0���� ũ�� transition�� t ����
			}
		}

		if (phones[p_index].name == "sp" && p > 0) { // voca�� phone�� sp�̰� phone index�� 0���� ũ��
			// If the last phone is "sp", then we can skip it
			int sp_index = p_index; // sp�� index ����
			double skipProb = phones[sp_index].tp[0][2]; // skip�Ҽ��ִ� transitional probability, 0(����) -> 2(����)

			int prev_p_index = *(phone_seq - 1); // sp ���� phone index ����
			int prev_p_n_state = getNumberOfPhoneState(prev_p_index); // �� phone�� state ���� ����

			int next_p_index = *(phone_seq + 1); // ���� phone index ����
			int next_p_n_state = getNumberOfPhoneState(next_p_index); // �� phone�� state ���� ����
			t.to_phone = p + 1;

			for (int prev_s = 0; prev_s < prev_p_n_state; prev_s++) { // �� state ����
				for (int next_s = 0; next_s < next_p_n_state; next_s++) { // ���� phone�� �� state����
					t.to_state = next_s; // ���� state ����
					t.to_prob = getEndingProb(prev_p_index, prev_s) * skipProb * getBeginningProb(next_p_index, next_s); // (���� phone, state���� Ȯ��) * (skip�� Ȯ��) * (bigram(����)(����)) * (���� phone,state���� Ȯ��) ����
					if (t.to_prob > 0) transitions[p - 1][prev_s].push_back(t); // Ȯ���� 0���� ũ�� transition�� t ����
				}
			}
		}
		p++;
	}

	p = 1;
	// case 2. phone from prev phone
	for (phone_seq = phone_seqs[tau].begin() + 1; phone_seq != phone_seqs[tau].end(); ++phone_seq) {
		int p_index = *phone_seq; // phone index ����
		int p_n_state = getNumberOfPhoneState(p_index); // �ش� phone���� state ��������
		int prev_p_index = *(phone_seq - 1); // ���� phone index ����
		int prev_p_n_state = getNumberOfPhoneState(prev_p_index); // ���� phone���� state ��������
		tPrev.from_phone = p - 1; // p - 1 �ε��� ����(���� phone)
		for (int s = 0; s < p_n_state; s++) { // �� state ����
			for (int prev_s = 0; prev_s < prev_p_n_state; prev_s++) { // �� ���� state ����
				tPrev.from_state = prev_s; // ���� state ����
				tPrev.from_prob = getEndingProb(prev_p_index, prev_s) * getBeginningProb(p_index, s); // (���� phone�� state�� Ȯ�� * ���� phone�� state�� Ȯ��)�� ����
				if (tPrev.from_prob > 0) transitionsPrev[p][s].push_back(tPrev); // �� Ȯ���� 0���� ũ�� transition�� t ����
			}
		}

		if (phones[p_index].name == "sp" && p > 0) { // voca�� phone�� sp�̰� phone index�� 0���� ũ��
			// If the last phone is "sp", then we can skip it
			int sp_index = p_index; // sp�� index ����
			double skipProb = phones[sp_index].tp[0][2]; // skip�Ҽ��ִ� transitional probability, 0(����) -> 2(����)

			int next_p_index = *(phone_seq + 1); // ���� phone index ����
			int next_p_n_state = getNumberOfPhoneState(next_p_index); // �� phone�� state ���� ����

			int prev_p_index = *(phone_seq - 1); // sp ���� phone index ����
			int prev_p_n_state = getNumberOfPhoneState(prev_p_index); // �� phone�� state ���� ����
			tPrev.from_phone = p - 1;

			for (int next_s = 0; next_s < next_p_n_state; next_s++) { // �� state ����
				for (int prev_s = 0; prev_s < prev_p_n_state; prev_s++) { // ���� phone�� �� state����
					tPrev.from_state = prev_s;
					tPrev.from_prob = getEndingProb(prev_p_index, prev_s) * skipProb * getBeginningProb(next_p_index, next_s); // (���� phone, state���� Ȯ��) * (skip�� Ȯ��) * (bigram(����)(����)) * (���� phone,state���� Ȯ��) ����
					if (tPrev.from_prob > 0) transitionsPrev[p + 1][next_s].push_back(tPrev); // Ȯ���� 0���� ũ�� transition�� t ����
				}
			}
		}
		p++;
	}

	applyLogScale(tau); // log scale�� Ȯ�� ��ȯ

}
