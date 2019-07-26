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
#include <regex>

#include "Judger_Function.h"
#include "JudgerType.h"
#include "Encode.h"

#include <MySql.h>
using namespace std;

//ÿ�����Ե������״̬��Ϣ
struct StatusData
{
	int status;
	int runTime;
	int timeUsed;
	int memoryUsed;
	int exitCode;

};
static StatusData testStatus[110];

//���Ե���Ϣ�����ڴ����߳���Ϣ
JudgeDate Test[110];

//����߳���
int ThreadNum = 1;
//��ʱ�ز����
int JudgeAgainNum = 1;
//���Ե�����
int TestNum = 1;
//ʹ������
static int language;
//�Ƿ�ɾ������ĩ�ո�
bool isRemoveBlank;

//ǿ��ֹͣ����
bool StopJudge = false;

//����·��
const char *g_CodePath[] = 
{"./test/%d/Code.c", 
 "./test/%d/Code.cpp"
};

//��������
const char *g_cplCommand[] =
{ "\"gcc\" \"%s\" -o \"./test/%d/Test.exe\" -O2 -Wall -lm --static -std=c99 -DONLINE_JUDGE",
  "\"g++\" \"%s\" -o \"./test/%d/Test.exe\" -O2 -Wall -lm --static -DONLINE_JUDGE",
  "\"javac\" -J-Xms32m -J-Xmx256m \"./test/%d/Main.java\"",
  "\"python\" -m py_compile \"./test/%d/Code.py\""
};

//��������
const char *g_runCommand[] =
{ "./test/%d/Test.exe",
  "./test/%d/Test.exe",
  "\"java\" -cp \"./test/%d\" \"Main\"",
  "\"python\" \"./test/%d/Code.py\""
};

funJudger_t::funJudger_t()
{
	//��ʼ���ṹ������
	Reset();

	ThreadNum = 1;
	TestNum = 1;

	runID = 1;
	problemNum = 1000;
	language = SE_CPlusPlus;
	timeLimit = 1000;
	memoryLimit = 65536;
}

funJudger_t::~funJudger_t()
{

}

bool funJudger_t::CheckCode()
{
	if (language != SE_C && language != SE_CPlusPlus)
		return true;

	char CodePath[100];
	if (language == SE_Java)
	{
		sprintf_s(CodePath, "test\\%d\\Main.java", runID);
	}
	else if (language == SE_Python)
	{
		sprintf_s(CodePath, "test\\%d\\Code.py", runID);
	}
	else if (language == SE_C)
	{
		sprintf_s(CodePath, "test\\%d\\Code.c", runID);
	}
	else
	{
		sprintf_s(CodePath, "test\\%d\\Code.cpp", runID);
	}

	ifstream infile;
	infile.open(CodePath);
	string Str;

	const string SensitiveStr[] = { ".*windows.*", ".*system.*", ".*ExitWindowsEx.*", ".*fopen.*", ".*ifstream.*", 
		".*WinExec.*", ".*ShellExecute.*", ".*ShellExecuteEx.*", ".*CreateProcess.*", ".*thread.*", 
		".*OpenProcess.*", ".*CloseHandle.*", ".*remove.*", ".*DeleteFile.*", ".*CreateDirectory.*",
		".*__stdcall.*", ".*GetProcAddress.*", ".*LoadLibrary.*", ".*RemoveDirectory.*"};

	int SensitiveStrNum = sizeof(SensitiveStr) / sizeof(SensitiveStr[0]);

	bool Find = false;

	while (getline(infile, Str))
	{
		
		for (int i = 0; i < SensitiveStrNum; i++)
		{
			regex reg(SensitiveStr[i], regex_constants::icase);
			bool bValid = regex_match(Str, reg);

			if (bValid)
			{
				printf("��⵽�ؼ��֣���ֹͣ����\n");
				Find = true;

				sprintf_s(CodePath, "Temporary_Error\\%d.log", runID);
				ofstream out(CodePath);
				if (out.is_open())
				{
					string iSensitiveStr = SensitiveStr[i];

					string noWord = ".*";
					size_t pos = iSensitiveStr.find(noWord);
					while (pos != -1)
					{
						iSensitiveStr.replace(pos, noWord.length(), "");
						pos = iSensitiveStr.find(noWord);
					}

					out << UnicodeToANSI(L"��⵽���д�: ") << iSensitiveStr.c_str() << endl;
					out << UnicodeToANSI(L"��������ֹ����ɾȥ���д��ٽ��в���.");
					out.close();
				}

				break;
			}
		}

		if (Find)
			break;
	}

	return !Find;
}

bool funJudger_t::Compile()
{
	if (!CheckCode())
	{
		return false;
	}

	MySQL_ChangeStatus(runID, Compiling);

	bool status = false;

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.bInheritHandle = TRUE;

	char PutPath[PATHLEN];
	sprintf_s(PutPath, "./Temporary_Error/%d.log", runID);
	HANDLE cmdError = CreateFile(PutPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	char CodePath[PATHLEN];
	sprintf_s(CodePath, g_CodePath[language], runID);

	//ִ�еı�������
	char command[PATHLEN];
	if(language == SE_Java || language == SE_Python)
		sprintf_s(command, g_cplCommand[language], runID);
	else
		sprintf_s(command, g_cplCommand[language], CodePath, runID);
	
	//״̬
	//Compiling;

	//�ض������������
	si.hStdError = cmdError;
	si.dwFlags = STARTF_USESTDHANDLES;

	DWORD exitcode;
	//��������
	if (CreateProcess(NULL, command, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		printf("������...\n");
		WaitForSingleObject(pi.hProcess, 60000);

		GetExitCodeProcess(pi.hProcess, &exitcode);

		if (exitcode != 0)
		{
			status = false;
			printf("���뷢�������˳��룺%x\n", exitcode);
			PrintErrorLog(runID);
		}
		else
		{
			status = true;
			printf("�������\n");
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

void funJudger_t::JudgerStop()
{
	StopJudge = true;
}

void funJudger_t::Reset()
{
	for (int i = 0; i < 100; i++)
	{
		testStatus[i].runTime = 0;

		testStatus[i].memoryUsed = 0;
		testStatus[i].timeUsed = 0;
		testStatus[i].status = 0;

		Test[i].memoryLimit = 0;
		Test[i].timeLimit = 0;
		Test[i].runID = 0;
		Test[i].testNum = 0;
		Test[i].problemNum = 0;
	}

	LastStatus = 0;
	LastTimeUsed = 0;
	LastMemoryUsed = 0;

	StopJudge = false;
}

void funJudger_t::SetLanguage(const char *Name)
{
	if (!strcmp(Name, "Java"))
	{
		language = SE_Java;
	}
	else if (!strcmp(Name, "Python"))
	{
		language = SE_Python;
	}
	else if (!strcmp(Name, "Gcc"))
	{
		language = SE_C;
	}
	else
	{
		language = SE_CPlusPlus;
	}
}

void funJudger_t::SetNumofThread(int num)
{
	printf("��������߳�����%d\n", num);
	ThreadNum = num;
}

void funJudger_t::SetNumofTimeLimit(int num)
{
	printf("���ó�ʱ�ز������%d\n", num);
	JudgeAgainNum = num;
}

void funJudger_t::SetJudgeMode(int mode)
{
	printf("��������ģʽ��%d\n", mode);
	judgeMode = mode;
}

void funJudger_t::SetRunID(int num)
{
	printf("��������ID��%d\n", num);
	runID = num;
}

void funJudger_t::SetTestNum(int num)
{
	printf("���ò��Ե�������%d\n", num);
	TestNum = num;
}

void funJudger_t::SetAllTestNum(int *allTest)
{
	printf("����ÿ�����Ե��\n");
	allTestNum = allTest;
}

void funJudger_t::SetMemoryLimit(int num)
{
	printf("�����ڴ����ƣ�%d\n", num);
	memoryLimit = num;
}
void funJudger_t::SetTimeLimit(int num)
{
	printf("����ʱ�����ƣ�%d\n", num);
	timeLimit = num;
}

void funJudger_t::SetProblemNum(int num)
{
	printf("������ţ�%d\n", num);
	problemNum = num;
}

void funJudger_t::SetRemoveBlank(bool Remove)
{
	printf("������������ʱ������ĩ�ո�ͻ��з�\n");
	isRemoveBlank = Remove;
}

void funJudger_t::PrintErrorLog(int RunID)
{
	char PutPath[PATHLEN];
	sprintf_s(PutPath, "./log/Error_%d.log", RunID);

	ifstream is(PutPath);

	char buf[1000];

	while (is.getline(buf, 1000))
	{
		wchar_t *wBuf = UTF8ToUnicode(buf);
		char *AnsiBuf = UnicodeToANSI(wBuf);

		printf("%s\n", AnsiBuf);
	}

	is.close();
}

void funJudger_t::CreateDir(int RunID)
{
	char Path[100];
	sprintf_s(Path, "test/%d", RunID);

	CreateDirectory(Path, NULL);
}

bool RemoveDir(const char* szFileDir)
{
	std::string strDir = szFileDir;

	if (strDir.at(strDir.length() - 1) != '\\')
		strDir += '\\';

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile((strDir + "*.*").c_str(), &wfd);

	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	do
	{
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (_stricmp(wfd.cFileName, ".") != 0 &&
				_stricmp(wfd.cFileName, "..") != 0)
				RemoveDir((strDir + wfd.cFileName).c_str());
		}
		else
		{
			bool Succeess = false;
			do
			{
				Succeess = DeleteFile((strDir + wfd.cFileName).c_str());

				if (!Succeess)
				{
					printf("�ļ���ռ�ã����ڳ���ɾ��...\n");
					Sleep(10);
				}
			} while (!Succeess);
		}
	} while (FindNextFile(hFind, &wfd));

	FindClose(hFind);
	RemoveDirectory(szFileDir);

	return true;
}

void funJudger_t::DeleteTestFile()
{
	char Path[PATHLEN];
	sprintf_s(Path, "test/%d", runID);

	RemoveDir(Path);
	//RemoveDirectory("test");
	/*
	char Path[PATHLEN];
	//ɾ��Ŀ¼�������ļ�
	sprintf_s(Path, "del /s /q \"test\\%d\\*.*\" >log\\Remove_%d.log", RunID, RunID);
	system(Path);
	//ɾ���ļ���
	sprintf_s(Path, "rd \"test\\%d\"", RunID);
	system(Path);*/
}

DWORD WINAPI funJudger_t::JudgeTest(LPVOID lpParamter)
{
	SetErrorMode(SEM_NOGPFAULTERRORBOX);

	JudgeDate *data = (JudgeDate *)lpParamter;

	char InputPath[PATHLEN];
	sprintf_s(InputPath, "./data/%d/%d_%d.in", data->problemNum, data->problemNum, data->testNum);

	char OutputPath[PATHLEN];
	sprintf_s(OutputPath, "./test/%d/Test_%d.out", data->runID, data->testNum);

	char ErrorPath[PATHLEN];
	sprintf_s(ErrorPath, "./Temporary_Error/Error_%d.log", data->runID);

	char ProgramPath[PATHLEN];
	sprintf_s(ProgramPath, g_runCommand[language], data->runID);

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

	if (!CreateProcess(NULL, ProgramPath, NULL, &sa, TRUE, HIGH_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		printf(" Test%2d: CreateProcessʧ��:0x%x\n", data->testNum, GetLastError());

		CloseHandle(si.hStdInput);
		CloseHandle(si.hStdOutput);
		CloseHandle(si.hStdError);

		testStatus[data->testNum].status = SystemError;
		return 0;
	}
	
	bool IsMemoryLimit = false;

	PROCESS_MEMORY_COUNTERS_EX info;
	ZeroMemory(&info, sizeof(info));
	info.cb = sizeof(info);

	GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS*)&info, sizeof(info));

	int maxMemory;// = (int)max(info.PrivateUsage, info.PeakWorkingSetSize) / 1024;

	if (language == SE_Java)
		maxMemory = (int)info.PeakWorkingSetSize / 1024;
	else
		maxMemory = (int)info.PeakPagefileUsage / 1024;

	bool IstimeLimit = true;

	if (maxMemory > data->memoryLimit)
	{
		IsMemoryLimit = true;
		IstimeLimit = false;
	}

	bool IsBreak = false;

	clock_t start, stop;
	int timeUsed = 0;
	start = clock();

	DWORD exitCode = 0;

	int runTime = 0;
	int extraTime = (int)ceil(max(2000, data->timeLimit * 2) * (0.1 * ThreadNum)) + data->timeLimit;

	if(data->resurvey)
		extraTime += (int)ceil(max(2000, data->timeLimit * 2) * (0.1 * ThreadNum));

	while (runTime <= extraTime)
	{
		if (StopJudge)
		{
			TerminateProcess(pi.hProcess, 0);

			CloseHandle(si.hStdInput);
			CloseHandle(si.hStdOutput);
			CloseHandle(si.hStdError);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);

			return 0;
		}

		//��������������������Ϊδ��ʱ����ѭ��
		if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0)
		{
			IstimeLimit = false;
		}

		stop = clock();
		runTime = (int)(stop - start);

		GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS*)&info, sizeof(info));

		if (language == SE_Java)
			maxMemory = (int)info.PeakWorkingSetSize / 1024;
		else
			maxMemory = (int)info.PeakPagefileUsage / 1024;

		//maxMemory = (int)max(info.PrivateUsage, info.PeakWorkingSetSize) / 1024;

		if (maxMemory > data->memoryLimit)
		{
			IsMemoryLimit = true;
			IstimeLimit = false;
		}

		if (!IstimeLimit || IsMemoryLimit)
		{
			IsBreak = true;

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

		Sleep(10);
	}

	//����ǳ����������ʱ���Զ��߳�ѭ���˾ͻ�ȡ����ʱ��
	if (IstimeLimit)
	{
		FILETIME creationTime, exitTime, kernelTime, userTime;
		GetProcessTimes(pi.hProcess, &creationTime, &exitTime, &kernelTime, &userTime);

		SYSTEMTIME realTime;
		FileTimeToSystemTime(&userTime, &realTime);

		timeUsed = realTime.wMilliseconds
			+ realTime.wSecond * 1000
			+ realTime.wMinute * 60 * 1000
			+ realTime.wHour * 60 * 60 * 1000;
	}//����ǳ����ڹ涨ʱ������������˾��жϻ�ȡ������ʱ���Ƿ�ʱ
	else if (timeUsed >= data->timeLimit)
	{
		IstimeLimit = true;
	}

	//��ֹ��Ϊ��ʱ����ѭ����û�н�������
	if (!IsBreak)
	{
		TerminateProcess(pi.hProcess, 0);

		CloseHandle(si.hStdInput);
		CloseHandle(si.hStdOutput);
		CloseHandle(si.hStdError);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	if (IsMemoryLimit)
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
	testStatus[data->testNum].timeUsed = timeUsed;
	testStatus[data->testNum].memoryUsed = maxMemory;
	testStatus[data->testNum].exitCode = exitCode;

	return 0;
}

void funJudger_t::PrintResult()
{
	printf("������ɣ�\n");
	GetResult();

	MySQL_ChangeStatus(runID, LastStatus);
	MySQL_SetUseTime(runID, LastTimeUsed);
	MySQL_SetUseMemory(runID, LastMemoryUsed);

	printf("Judge Over, %-22s  usetime:%-4dms  usememory:%-5dkb\n", ProgramStateStr[LastStatus], LastTimeUsed, LastMemoryUsed);
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

	printf("��ʼ���⣺ %d-%02d-%02d %02d:%02d:%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

	bool createThreadOver = false;

	int startTest = 1;
	int endTest = min(ThreadNum, TestNum);

	printf("��ʼ����%d - %d���Ե�\n", startTest, endTest);
	CreateTestThread(startTest, endTest);

	int timeLimitJudgeAgain[100];
	memset(timeLimitJudgeAgain, 0, sizeof timeLimitJudgeAgain);

	bool EncoError = false;

	while (1)
	{
		bool judgeOver = true;
		
		//����Ƿ����в��Ե㶼�������
		for (int i = startTest; i <= endTest; i++)
		{
			int iTestNum = allTestNum[i];

			if (testStatus[iTestNum].status == Null)
			{
				judgeOver = false;
				break;
			}
			else if (testStatus[iTestNum].status == TimeLimitExceeded && timeLimitJudgeAgain[iTestNum] < JudgeAgainNum)
			{
				timeLimitJudgeAgain[iTestNum]++;
				printf("@��ʱ�ز�[%d] �� ���Ե�%d\n", timeLimitJudgeAgain[iTestNum], iTestNum);

				testStatus[iTestNum].status = Null;
				CreateTestThread(i, true);
				judgeOver = false;

				//breakĿ���ǵȴ����߳�ִ����ɣ�ȥ�����̶߳������Ӱ��
				break;
			}

			if (judgeMode == Judge_ErrorStop)
			{
				if (testStatus[iTestNum].status == PresentationError ||
					testStatus[iTestNum].status == TimeLimitExceeded ||
					testStatus[iTestNum].status == MemoryLimitExceeded ||
					testStatus[iTestNum].status == WrongAnswer ||
					testStatus[iTestNum].status == RuntimeError ||
					testStatus[iTestNum].status == OutputLimitExceeded ||
					testStatus[iTestNum].status == SystemError)
				{
					printf("����������ֹͣ����\n");
					EncoError = true;
					StopJudge = true;

					break;
				}
			}
		}

		if (EncoError)
		{
			PrintResult();
			break;
		}

		if (judgeOver)
		{
			if (endTest < TestNum)
			{
				startTest += ThreadNum;
				endTest = min(TestNum, endTest + ThreadNum);

				printf("��ʼ����%d - %d���Ե�\n", startTest, endTest);
				CreateTestThread(startTest, endTest);
				continue;
			}


			PrintResult();
			break;
		}
	}

	proTimeUsed = (int)(clock() - start);
	printf("�����ܺ�ʱ��%dms\n\n", proTimeUsed);

	return 1;
}

void funJudger_t::CreateTestThread(int start, int end)
{
	//���̲߳������в��Ե�
	for (int i = start; i <= end; i++)
	{
		Test[i].runID = runID;
		Test[i].problemNum = problemNum;
		Test[i].testNum = allTestNum[i];
		Test[i].memoryLimit = memoryLimit;
		Test[i].timeLimit = timeLimit;
		Test[i].resurvey = false;

		CreateThread(NULL, 0, JudgeTest, &Test[i], 0, NULL);
	}
}

void funJudger_t::CreateTestThread(int num, bool resurvey)
{
	Test[num].runID = runID;
	Test[num].problemNum = problemNum;
	Test[num].testNum = allTestNum[num];
	Test[num].memoryLimit = memoryLimit;
	Test[num].timeLimit = timeLimit;
	Test[num].resurvey = resurvey;

	CreateThread(NULL, 0, JudgeTest, &Test[num], 0, NULL);
}

void funJudger_t::GetResult()
{
	//״̬�����ʾ���ȼ�
	//1.System Error
	//2.Runtime Error
	//3.Memory Limit Exceeded
	//4.Time Limit Exceeded
	//5.Wrong Answer
	//6.Output Limit Exceeded
	//7.Presentation Error
	//8.Accepted

	char AllStatus[1000] = "";

	for (int i = 1; i <= TestNum; i++)
	{
		int iTestNum = allTestNum[i];

		LastTimeUsed = max(LastTimeUsed, testStatus[iTestNum].timeUsed);
		LastMemoryUsed = max(LastMemoryUsed, testStatus[iTestNum].memoryUsed);

		if (testStatus[iTestNum].status == SystemError)
		{
			LastStatus = SystemError;
		}
		else if (testStatus[iTestNum].status == RuntimeError)
		{
			if (LastStatus != SystemError)
			{
				LastStatus = RuntimeError;
			}
		}
		else if (testStatus[iTestNum].status == TimeLimitExceeded || testStatus[iTestNum].status == MemoryLimitExceeded)
		{
			if (LastStatus != RuntimeError && LastStatus != SystemError)
			{
				LastStatus = testStatus[iTestNum].status;
			}
		}
		else if (testStatus[iTestNum].status == WrongAnswer)
		{
			if (LastStatus == Null || LastStatus == Accepted || LastStatus == PresentationError || LastStatus == OutputLimitExceeded)
			{
				LastStatus = WrongAnswer;
			}
		}
		else if (testStatus[iTestNum].status == PresentationError)
		{
			if (LastStatus == Null || LastStatus == Accepted)
			{
				LastStatus = PresentationError;
			}
		}
		else if (testStatus[iTestNum].status == OutputLimitExceeded)
		{
			if (LastStatus == Null || LastStatus == PresentationError || LastStatus == Accepted)
			{
				LastStatus = OutputLimitExceeded;
			}
		}
		else if (testStatus[iTestNum].status == Accepted)
		{
			if (LastStatus == Null)
			{
				LastStatus = Accepted;
			}
		}

		char iStatusStr[100];
		sprintf_s(iStatusStr, "%d&%d&%d&%d&0x%x|", iTestNum, testStatus[iTestNum].status - 1, testStatus[iTestNum].timeUsed, testStatus[iTestNum].memoryUsed, testStatus[iTestNum].exitCode);
		strcat_s(AllStatus, iStatusStr);

		printf("##%-3d: %-22s  usetime:%-4dms  runttime:%-4dms  usememory:%-5dkb  ExitCode:0x%x\n", iTestNum, ProgramStateStr[testStatus[iTestNum].status], testStatus[iTestNum].timeUsed, testStatus[iTestNum].runTime, testStatus[iTestNum].memoryUsed, testStatus[iTestNum].exitCode);
	}

	if (LastStatus != CompileError && LastStatus != SystemError)
	{
		int Strlen = (int)strnlen_s(AllStatus, 1000);

		if (Strlen >= 1)
			AllStatus[Strlen - 1] = '\0';

		MySQL_SetAllStatus(runID, AllStatus);
	}
}

void RemoveStringBlank(string &Str)
{
	if (!Str.empty())
	{
		Str.erase(0, Str.find_first_not_of(" "));
		Str.erase(Str.find_last_not_of(" ") + 1);
	}
}

//AC�������
bool funJudger_t::AcceptedTest(const char* program, const char* tester)
{
	ifstream is1(program);
	ifstream is2(tester);

	bool Res = true;

	string buf1;
	string buf2;

	while (getline(is2, buf2))
	{
		if (isRemoveBlank)
		{
			RemoveStringBlank(buf2);

			if (buf2 == "")
				continue;
		}

		if (!getline(is1, buf1))
		{
			Res = false;
			break;
		}

		if (isRemoveBlank)
		{
			RemoveStringBlank(buf1);
		}

		if (buf1 != buf2)
		{
			Res = false;
			break;
		}
	}

	if (getline(is1, buf1))
	{
		Res = false;
	}

	is1.close();
	is2.close();

	return Res;
}

//PE�������
bool funJudger_t::PresentationErrorTest(const char* program, const char* tester)
{
	ifstream is1(program);
	ifstream is2(tester);

	string buf1;
	string buf2;

	bool Res = true;

	while (is2 >> buf2)
	{
		if (!(is1 >> buf1))
		{
			Res = false;
			break;
		}

		if (buf1 != buf2)
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