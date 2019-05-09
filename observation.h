#ifndef OBSERVATION_H_
#define OBSERVATION_H_

// �Լ�����
// get log observation probability of spectrum vector
double getObservationProb(int phone, int state, double spectrum[]);
double probForOnePDF(int phone, int state, int pdf, double spectrum[]);

#endif