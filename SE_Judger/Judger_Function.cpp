#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <Psapi.h>
#include <time.h>
#include <string.h>

#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>

#include "Judger_Function.h"

using namespace std;

//每个测试点的状态信息
struct StatusData
{
	int status;
	int runTime;
	int timeUsed;
	int memoryUsed;
	int exitCode;

};
static StatusData testStatus[100];

//测试点信息
JudgeDate Test[110];
//最多线程数
int ThreadNum = 1;
//测试点数量
int TestNum = 1;
//程序语言
int Language;

//代码路径
const char *g_CodePath[] = 
{"./temporary/%d/Code.c", 
 "./temporary/%d/Code.cpp",
 "./temporary/%d/Main.java"
};

//编译命令
const char *g_cplCommand[] =
{ "\"gcc\" \"%s\" -o \"./temporary/%d/Test.exe\" -O2 -Wall -lm --static -std=c99 -DONLINE_JUDGE",
  "\"g++\" \"%s\" -o \"./temporary/%d/Test.exe\" -O2 -Wall -lm --static -DONLINE_JUDGE",
  "\"javac\" -J-Xms32m -J-Xmx256m \"./temporary/%d/Main.java\""
};

//运行命令
const char *g_runCommand[] =
{ "./temporary/%d/Test.exe",
  "./temporary/%d/Test.exe",
  "\"java\" -cp \"./temporary/%d\" \"Main\"",
  "\"python\" \"./temporary/%d/Code.py\""
};

funJudger_t::funJudger_t()
{
	for (int i = 0; i < 100; i++)
	{
		testStatus[i].memoryUsed = 0;
		testStatus[i].timeUsed = 0;
		testStatus[i].status = 0;
	}

	LastStatus = 0;
	LastTimeUsed = 0;
	LastMemoryUsed = 0;

	ThreadNum = 1;
	TestNum = 1;

	runID = 1;
	problemNum = 1000;
	language = SE_CPlusPlus;
	timeLimit = 1000;
	memoryLimit = 65536;

	Language = SE_CPlusPlus;
}

funJudger_t::~funJudger_t()
{

}

bool funJudger_t::Compile()
{
	if (language == SE_Python)
		return true;

	bool status = false;

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.bInheritHandle = TRUE;

	char PutPath[PATHLEN];
	sprintf_s(PutPath, "./log/Error_%d.log", runID);
	HANDLE cmdError = CreateFile(PutPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	char CodePath[PATHLEN];
	sprintf_s(CodePath, g_CodePath[language], runID);

	//执行的编译命令
	char command[PATHLEN];
	if(language == SE_Java)
		sprintf_s(command, g_cplCommand[language], runID);
	else
		sprintf_s(command, g_cplCommand[language], CodePath, runID);
	
	//状态
	//Compiling;

	//重定向编译错误输出
	si.hStdError = cmdError;
	si.dwFlags = STARTF_USESTDHANDLES;

	DWORD exitcode;
	//创建进程
	if (CreateProcess(NULL, command, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		printf("编译中...\n");
		WaitForSingleObject(pi.hProcess, INFINITE);

		GetExitCodeProcess(pi.hProcess, &exitcode);

		if (exitcode != 0)
		{
			status = false;
			printf("编译发生错误\n");
		}
		else
		{
			status = true;
			printf("编译完成\n");
		}
	}
	else
	{
		status = false;
	}

	TerminateProcess(pi.hProcess, 0);

	CloseHandle(si.hStdError);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return status;
}

void funJudger_t::SetLanguage(const char *Name)
{
	if (!strcmp(Name, "Java"))
	{
		Language = SE_Java;
		language = SE_Java;
	}
	else if (!strcmp(Name, "Python"))
	{
		Language = SE_Python;
		language = SE_Python;
	}
	else if (!strcmp(Name, "Gcc"))
	{
		Language = SE_C;
		language = SE_C;
	}
	else
	{
		Language = SE_CPlusPlus;
		language = SE_CPlusPlus;
	}
}

void funJudger_t::SetNumofThread(int num)
{
	printf("设置最大线程数：%d\n", num);
	ThreadNum = num;
}

void funJudger_t::SetRunID(int num)
{
	printf("设置运行ID：%d\n", num);
	runID = num;
}

void funJudger_t::SetTestNum(int num)
{
	printf("设置测试点数量：%d\n", num);
	TestNum = num;
}

void funJudger_t::SetAllTestNum(int *allTest)
{
	printf("设置每个测试点号\n");
	allTestNum = allTest;
}

void funJudger_t::SetMemoryLimit(int num)
{
	printf("设置内存限制：%d\n", num);
	memoryLimit = num;
}
void funJudger_t::SetTimeLimit(int num)
{
	printf("设置时间限制：%d\n", num);
	timeLimit = num;
}

void funJudger_t::SetProblemNum(int num)
{
	printf("设置题号：%d\n", num);
	problemNum = num;
}

void funJudger_t::DeleteTestFile(int RunID)
{
	char Path[PATHLEN];
	//删除目录下所有文件
	sprintf_s(Path, "del /s /q \"temporary\\%d\\*.out\" >log\\Remove_%d.log", RunID, RunID);
	system(Path);
	//删除文件夹
	//sprintf_s(Path, "rd \"test\\%d\"", RunID);
	//system(Path);
}

DWORD WINAPI funJudger_t::JudgeTest(LPVOID lpParamter)
{
	JudgeDate *data = (JudgeDate *)lpParamter;

	char InputPath[PATHLEN];
	sprintf_s(InputPath, "./data/%d/%d_%d.in", data->problemNum, data->problemNum, data->testNum);

	char OutputPath[PATHLEN];
	sprintf_s(OutputPath, "./temporary/%d/Test_%d.out", data->runID, data->testNum);

	char ErrorPath[PATHLEN];
	sprintf_s(ErrorPath, "./log/Error_%d.log", data->runID);

	char ProgramPath[PATHLEN];
	sprintf_s(ProgramPath, g_runCommand[Language], data->runID);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES sa;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&sa, sizeof(sa));
	sa.bInheritHandle = TRUE;

	si.hStdInput =	CreateFile((LPCSTR)InputPath,	GENERIC_READ,  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa, OPEN_ALWAYS,   FILE_ATTRIBUTE_NORMAL, NULL);
	si.hStdOutput = CreateFile((LPCSTR)OutputPath,	GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	si.hStdError =	CreateFile((LPCSTR)ErrorPath,	GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	//DWORD createFlag = CREATE_SUSPENDED | CREATE_NO_WINDOW | CREATE_BREAKAWAY_FROM_JOB;
	DWORD createFlag = HIGH_PRIORITY_CLASS | CREATE_NO_WINDOW;

	if (!CreateProcess(NULL, ProgramPath, NULL, &sa, TRUE, createFlag, NULL, NULL, &si, &pi))
	{
		printf(" Test%2d: CreateProcess失败:0x%x\n", data->testNum, GetLastError());

		CloseHandle(si.hStdInput);
		CloseHandle(si.hStdOutput);
		CloseHandle(si.hStdError);

		testStatus[data->testNum].status = SystemError;
		return 0;
	}
	
	bool memoryLimit = false;

	PROCESS_MEMORY_COUNTERS_EX info;
	ZeroMemory(&info, sizeof(info));
	info.cb = sizeof(info);

	GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS*)&info, sizeof(info));

	int maxMemory = max(info.PrivateUsage, info.PeakWorkingSetSize) / 1024;

	bool IstimeLimit = true;

	if (maxMemory > data->memoryLimit)
	{
		memoryLimit = true;
		IstimeLimit = false;
	}

	bool IsBreak = false;

	clock_t start, stop;
	int timeUsed = 0;
	start = clock();

	DWORD exitCode = 0;

	int runTime = 0;
	int extraTime = (int)ceil(max(2000, data->timeLimit * 2) * (0.1 * ThreadNum));

	do
	{
		if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0)
		{
			IstimeLimit = false;
		}

		stop = clock();
		runTime = (int)(stop - start);

		GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS*)&info, sizeof(info));
		maxMemory = max(info.PrivateUsage, info.PeakWorkingSetSize) / 1024;

		if (maxMemory > data->memoryLimit)
		{
			memoryLimit = true;
		}

		if (!IstimeLimit || memoryLimit)
		{
			IsBreak = true;

			if (memoryLimit)
			{
				//超内存后设置为未超时，获取进程使用时间
				IstimeLimit = false;
			}
			if (!IstimeLimit)
			{
				FILETIME creationTime, exitTime, kernelTime, userTime;
				GetProcessTimes(pi.hProcess, &creationTime, &exitTime, &kernelTime, &userTime);

				SYSTEMTIME realTime;
				FileTimeToSystemTime(&userTime, &realTime);

				timeUsed = realTime.wMilliseconds
					+ realTime.wSecond * 1000
					+ realTime.wMinute * 60 * 1000
					+ realTime.wHour * 60 * 60 * 1000;
			}

			GetExitCodeProcess(pi.hProcess, &exitCode);

			TerminateProcess(pi.hProcess, 0);

			CloseHandle(si.hStdInput);
			CloseHandle(si.hStdOutput);
			CloseHandle(si.hStdError);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);

			break;
		}

	} while (runTime <= (data->timeLimit + extraTime));

	if (!IstimeLimit && timeUsed > data->timeLimit)
	{
		IstimeLimit = true;
	}

	//防止因为超时跳出循环而没有结束进程
	if (!IsBreak)
	{
		TerminateProcess(pi.hProcess, 0);

		CloseHandle(si.hStdInput);
		CloseHandle(si.hStdOutput);
		CloseHandle(si.hStdError);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	if (memoryLimit)
	{
		testStatus[data->testNum].status = MemoryLimitExceeded;
	}
	else if (IstimeLimit)
	{
		testStatus[data->testNum].status = TimeLimitExceeded;
	}
	else
	{
		char ProgramOutputPath[PATHLEN];
		sprintf_s(ProgramOutputPath, "./data/%d/%d_%d.out", data->problemNum, data->problemNum, data->testNum);

		if (exitCode == 0)
		{
			if (AcceptedTest(ProgramOutputPath, OutputPath))
			{
				testStatus[data->testNum].status = Accepted;
			}
			else if (PresentationErrorTest(ProgramOutputPath, OutputPath))
			{
				testStatus[data->testNum].status = PresentationError;
			}
			else if (OutputLimitExceededTest(ProgramOutputPath, OutputPath))
			{
				testStatus[data->testNum].status = OutputLimitExceeded;
			}
			else
			{
				testStatus[data->testNum].status = WrongAnswer;
			}
		}
		else
		{
			testStatus[data->testNum].status = RuntimeError;
		}
	}

	testStatus[data->testNum].runTime = runTime;
	testStatus[data->testNum].timeUsed = IstimeLimit ? data->timeLimit : timeUsed;
	testStatus[data->testNum].memoryUsed = maxMemory;
	testStatus[data->testNum].exitCode = exitCode;

	return 0;
}

int funJudger_t::Run()
{
	clock_t start;
	int proTimeUsed = 0;
	start = clock();
	/*char LogPath[100];
	sprintf_s(LogPath, "./log/Judger_%d.log", 1);
	FILE *stream;
	freopen_s(&stream, LogPath, "w", stdout);*/

	time_t tt = time(NULL);
	tm* t = new tm;
	localtime_s(t, &tt);

	printf("开始评测： %d-%02d-%02d %02d:%02d:%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

	bool createThreadOver = false;

	int startTest = 1;
	int endTest = min(ThreadNum, TestNum);

	printf("开始评测%d - %d测试点\n", startTest, endTest);
	CreateTestThread(startTest, endTest);

	while (1)
	{
		bool judgeOver = true;

		//检查是否所有测试点都运行完毕
		for (int i = startTest; i <= endTest; i++)
		{
			if (testStatus[i].status == 0)
			{
				judgeOver = false;
				break;
			}
		}

		if (judgeOver)
		{
			if (endTest < TestNum)
			{
				startTest += ThreadNum;
				endTest = min(TestNum, endTest + ThreadNum);

				printf("开始评测%d - %d测试点\n", startTest, endTest);
				CreateTestThread(startTest, endTest);
				continue;
			}

			printf("测试点 结果      时间            Judge时间        内存               退出码\n");
			GetResult();

			DeleteTestFile(runID);

			printf("\nJudge Over, %s  usetime:%4dms  usememory:%5dkb\n", ProgramStateStr[LastStatus], LastTimeUsed, LastMemoryUsed);
			break;
		}
	}

	proTimeUsed = (int)(clock() - start);
	printf("评测总耗时：%dms\n", proTimeUsed);
	return 1;
}

void funJudger_t::CreateTestThread(int start, int end)
{
	//多线程并行运行测试点
	for (int i = start; i <= end; i++)
	{
		Test[i].runID = runID;
		Test[i].problemNum = problemNum;
		Test[i].testNum = allTestNum[i];
		Test[i].memoryLimit = memoryLimit;
		Test[i].timeLimit = timeLimit;

		CreateThread(NULL, 0, JudgeTest, &Test[i], 0, NULL);
	}
}

void funJudger_t::GetResult()
{
	//状态结果显示优先级
	//1.System Error
	//2.Runtime Error
	//3.Memory Limit Exceeded
	//4.Time Limit Exceeded
	//5.Wrong Answer
	//6.Output Limit Exceeded
	//7.Presentation Error
	//8.Accepted
	for (int i = 1; i <= TestNum; i++)
	{
		LastTimeUsed = max(LastTimeUsed, testStatus[i].timeUsed);
		LastMemoryUsed = max(LastMemoryUsed, testStatus[i].memoryUsed);

		if (testStatus[i].status == SystemError)
		{
			LastStatus = SystemError;
		}
		else if (testStatus[i].status == RuntimeError)
		{
			if (LastStatus != SystemError)
			{
				LastStatus = RuntimeError;
			}
		}
		else if (testStatus[i].status == TimeLimitExceeded || testStatus[i].status == MemoryLimitExceeded)
		{
			if (LastStatus != RuntimeError && LastStatus != SystemError)
			{
				LastStatus = testStatus[i].status;
			}
		}
		else if (testStatus[i].status == WrongAnswer)
		{
			if (LastStatus == 0 || LastStatus == Accepted || LastStatus == PresentationError || LastStatus == OutputLimitExceeded)
			{
				LastStatus = WrongAnswer;
			}
		}
		else if (testStatus[i].status == PresentationError)
		{
			if (LastStatus == 0)
			{
				LastStatus = PresentationError;
			}
		}
		else if (testStatus[i].status == OutputLimitExceeded)
		{
			if (LastStatus == 0 || LastStatus == PresentationError)
			{
				LastStatus = OutputLimitExceeded;
			}
		}
		else if (testStatus[i].status == Accepted)
		{
			if (LastStatus == 0)
			{
				LastStatus = Accepted;
			}
		}

		printf("##%2d : %s  usetime:%4dms  runttime:%4dms  usememory:%5dkb  ExitCode:%d\n", i, ProgramStateStr[testStatus[i].status], testStatus[i].timeUsed, testStatus[i].runTime, testStatus[i].memoryUsed, testStatus[i].exitCode);
	}
}

//AC结果评测
bool funJudger_t::AcceptedTest(const char* program, const char* tester)
{
	ifstream is1(program);
	ifstream is2(tester);

	bool Res = true;

	char buf1[1000];
	char buf2[1000];

	while (is2.getline(buf2, 1000))
	{
		if (!is1.getline(buf1, 1000))
		{
			Res = false;
			break;
		}

		if (strcmp(buf1, buf2) != 0)
		{
			Res = false;
			break;
		}
	}

	if (is1.getline(buf1, 1000))
	{
		Res = false;
	}

	is1.close();
	is2.close();

	return Res;
}

//PE结果评测
bool funJudger_t::PresentationErrorTest(const char* program, const char* tester)
{
	ifstream is1(program);
	ifstream is2(tester);

	char buf1[1000];
	char buf2[1000];

	bool Res = true;

	while (is2 >> buf2)
	{
		if (!(is1 >> buf1))
		{
			Res = false;
			break;
		}

		if (strcmp(buf1, buf2) != 0)
		{
			Res = false;
			break;
		}
	}

	if (is1 >> buf1)
	{
		Res = false;
	}

	is1.close();
	is2.close();

	return Res;
}

bool funJudger_t::OutputLimitExceededTest(const char* program, const char* tester)
{
	ifstream is1(program);
	ifstream is2(tester);

	char buf1[1000];
	char buf2[1000];

	bool Res = true;

	while (is1 >> buf1)
	{
		if (!(is2 >> buf2))
		{
			Res = false;
			break;
		}
	}

	if (!(is2 >> buf2) && Res)
	{
		Res = false;
	}

	is1.close();
	is2.close();

	return Res;
}