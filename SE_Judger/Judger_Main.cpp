#include <stdio.h>

#include "Judger_Function.h"

int main()
{
	printf("已启动  代码评测机\n");
	funJudger_t *Judger = new funJudger_t;

	Judger->SetNumofThread(5);
	Judger->SetNumofTimeLimit(1);
	Judger->SetRemoveBlank(true);
	Judger->SetJudgeMode(Judge_ErrorStop);

	int allTest[100];
	for (int i = 1; i <= 10; i++)
	{
		allTest[i] = i;
	}

	Judger->SetRunID(2);
	Judger->SetLanguage("C++");
	if (Judger->Compile())
	{
		Judger->SetTestNum(10);
		Judger->SetAllTestNum(allTest);
		Judger->SetProblemNum(1000);
		Judger->SetTimeLimit(1000);
		Judger->SetMemoryLimit(65536);
		Judger->Run();
	}


	Judger->Reset();
	Judger->SetRunID(5);
	Judger->SetLanguage("Java");
	if (Judger->Compile())
	{
		Judger->SetTestNum(10);
		Judger->SetAllTestNum(allTest);
		Judger->SetProblemNum(1000);
		Judger->SetTimeLimit(1000);
		Judger->SetMemoryLimit(65536);
		Judger->Run();
	}



	Judger->Reset();
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
		Judger->SetTimeLimit(1500);
		Judger->SetMemoryLimit(65536);
		Judger->Run();
	}

	delete Judger;
	system("pause");

	return 0;
}