#include <stdio.h>

#include "Judger_Function.h"

int main()
{
	funJudger_t *Judger = new funJudger_t;

	Judger->SetNumofThread(5);

	int allTest[100];
	for (int i = 1; i <= 35; i++)
	{
		allTest[i] = i;
	}

	Judger->SetRunID(1);
	Judger->SetLanguage("Python");

	if (Judger->Compile())
	{
		Judger->SetTestNum(10);
		Judger->SetAllTestNum(&allTest[0]);
		Judger->SetProblemNum(1000);
		Judger->SetTimeLimit(1000);
		Judger->SetMemoryLimit(65536);
		Judger->Run();
	}

	system("pause");
	delete Judger;

	return 0;
}