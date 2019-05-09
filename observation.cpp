#include <math.h>
#include <iostream>
#include "phone.h"
#include "observation.h"


// 
// get log probability for one probability density function. O(N_DIMENSION)
double probForOnePDF(int phone, int state, int pdf, double spectrum[]) {
    double sum = 0;
    double * mean = phones[phone].state[state].pdf[pdf].mean; // phones�� �� sate���� �� pdf�� mean�迭�� �����ͺ��� mean�� �Ҵ�
    double * var = phones[phone].state[state].pdf[pdf].var; // phones�� �� sate���� �� pdf�� var�迭�� �����ͺ��� var�� �Ҵ�

    for (int d = 0; d < N_DIMENSION; d++) { // �� pdf(Ȯ���е��Լ�, ����þȺ���)���� Ȯ���� ����, �ϳ��� pdf�� ���Ժ����� Ȯ���� ����Ʈ������ Ȯ�������� �����־����.
        sum -= log(var[d])/2.0 + (spectrum[d] - mean[d]) * (spectrum[d] - mean[d]) / (2 * var[d]); //  log�� �������Ƿ� ���ϴ� ������ ��. (2pi)^(D/2)�� ���������� �ִ� ���̹Ƿ� ����. (����ϴ� ������ �ִ��� ���ϴ� ���̹Ƿ�)
    }

    return sum;
}

// O(N_DIMENSION * N_PDF)
double getObservationProb(int phone, int state, double spectrum[]) { // B(observation probability)�� ����
    double probs[N_PDF]; // pdf1, pdf2�� Ȯ��

    for (int pdf = 0; pdf < N_PDF; pdf++) {
        probs[pdf] = probForOnePDF(phone, state, pdf, spectrum); // pdf �ϳ��� Ȯ���� probs�� ����
    }

    // probs divided by first prob (not log scale)
    double dividedProbs[N_PDF];
    for (int pdf = 0; pdf < N_PDF; pdf++) {
        dividedProbs[pdf] = exp(probs[pdf] - probs[0]); // e^(li-l1)�� ���
    }

    double sum = 0;
    for (int pdf = 0; pdf < N_PDF; pdf++) {
        // multiply weight and add to sum
        sum += dividedProbs[pdf] * phones[phone].state[state].pdf[pdf].weight; // weight * dividedeProbs�� ���� ����
    }

    return probs[0] + log(sum); // ���� observation prob�� �����.
}
