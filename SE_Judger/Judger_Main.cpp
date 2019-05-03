#include <stdio.h>

#include "Judger_Function.h"

int main()
{
	funJudger_t *Judger = new funJudger_t;

	Judger->SetNumofThread(5);
	Judger->SetNumofTimeLimit(2);

	int allTest[100];
	allTest[1] = 1;
	allTest[2] = 2;
	allTest[3] = 6;
	allTest[4] = 7;
	allTest[5] = 8;

	Judger->SetRunID(3);
	Judger->SetLanguage("C++");

	if (Judger->Compile())
	{
		Judger->SetTestNum(5);
		Judger->SetAllTestNum(allTest);
		Judger->SetProblemNum(1006);
		Judger->SetTimeLimit(1000);
		Judger->SetMemoryLimit(65536);
		Judger->Run();
	}

	delete Judger;
	system("pause");

	return 0;
}