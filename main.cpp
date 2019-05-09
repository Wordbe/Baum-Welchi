#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include "transition.h"
#include "baumwelch.h"
#include "fileutil.h"
#include <math.h>

using namespace std;



void runAllTests() {

	//ReadIntputFile();
	runBaumWelch();
	writeOutputFile();

	
	printf("100%%\n"); // �˰����� �Ϸ�Ǹ� 100% ���
}

int main() {

	readTranscription();
	runAllTests(); // �ٿ�-��ġ �˰����� �����Ű�鼭 ����� ���ο� ���� hmm.txt�� ����

	return 0;
}