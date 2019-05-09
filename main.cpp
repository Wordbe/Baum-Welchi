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

	
	printf("100%%\n"); // 알고리즘이 완료되면 100% 출력
}

int main() {

	readTranscription();
	runAllTests(); // 바움-웰치 알고리즘을 실행시키면서 결과값 새로운 모델을 hmm.txt에 저장

	return 0;
}