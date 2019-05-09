#include <math.h>
#include <iostream>
#include "phone.h"
#include "observation.h"


// 
// get log probability for one probability density function. O(N_DIMENSION)
double probForOnePDF(int phone, int state, int pdf, double spectrum[]) {
    double sum = 0;
    double * mean = phones[phone].state[state].pdf[pdf].mean; // phones의 각 sate마다 각 pdf의 mean배열을 포인터변수 mean에 할당
    double * var = phones[phone].state[state].pdf[pdf].var; // phones의 각 sate마다 각 pdf의 var배열을 포인터변수 var에 할당

    for (int d = 0; d < N_DIMENSION; d++) { // 각 pdf(확률밀도함수, 가우시안분포)마다 확률을 얻음, 하나의 pdf의 정규분포의 확률은 스펙트럼마다 확률분포를 곱해주어야함.
        sum -= log(var[d])/2.0 + (spectrum[d] - mean[d]) * (spectrum[d] - mean[d]) / (2 * var[d]); //  log를 취했으므로 합하는 형식이 됨. (2pi)^(D/2)는 공통적으로 있는 값이므로 생략. (비례하는 값으로 최댓값을 비교하는 것이므로)
    }

    return sum;
}

// O(N_DIMENSION * N_PDF)
double getObservationProb(int phone, int state, double spectrum[]) { // B(observation probability)를 구함
    double probs[N_PDF]; // pdf1, pdf2의 확률

    for (int pdf = 0; pdf < N_PDF; pdf++) {
        probs[pdf] = probForOnePDF(phone, state, pdf, spectrum); // pdf 하나의 확률을 probs에 저장
    }

    // probs divided by first prob (not log scale)
    double dividedProbs[N_PDF];
    for (int pdf = 0; pdf < N_PDF; pdf++) {
        dividedProbs[pdf] = exp(probs[pdf] - probs[0]); // e^(li-l1)을 계산
    }

    double sum = 0;
    for (int pdf = 0; pdf < N_PDF; pdf++) {
        // multiply weight and add to sum
        sum += dividedProbs[pdf] * phones[phone].state[state].pdf[pdf].weight; // weight * dividedeProbs의 합을 구함
    }

    return probs[0] + log(sum); // 구한 observation prob의 결과값.
}
