#ifndef PHONE_H_
#define PHONE_H_

#define N_PHONE		21
#define N_STATE		3
#define N_PDF		2
#define N_DIMENSION	39

#include <string>

using namespace std;

// pdf타입 구조체
typedef struct {
  double weight;
  double mean[N_DIMENSION];
  double var[N_DIMENSION];
} pdfType;

// state타입 구조체
typedef struct {
  pdfType pdf[N_PDF];
} stateType;

// phone타입 구조체
typedef struct {
  string name;
  double tp[N_STATE+2][N_STATE+2];
  stateType state[N_STATE];
} phoneType;

// phone.cpp에서 phoneType구조체형 변수(배열) phones를 가져옴.
extern phoneType phones[N_PHONE];

// 함수 선언

int getPhoneIndex(string name);
int getNumberOfPhoneState(int index);

#endif