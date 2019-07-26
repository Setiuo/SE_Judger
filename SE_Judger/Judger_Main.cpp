#include <stdio.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <windows.h>
#include <conio.h>
#include <io.h>

#include <MySql.h>

#include "Encode.h"
#include "Judger_Function.h"
#include "JudgerType.h"

using namespace std;

struct FileData_t
{
	int RunID;
	char Data[200];

public:
	friend bool operator<(const FileData_t &a, const FileData_t &b)
	{
		return a.RunID >= b.RunID;
	}
};

char *FileDP;

funJudger_t JudgeSystem;

void Judge(funJudger_t &JudgeSystem, int RunID, int JudgeType, const char *UserName, const char *Problem, int MaxTime, int MaxMemory, const char *Lang, const char *Test)
{
	fprintf(stdout, "运行ID:%-3d 用户:%-5s 题号:%4s 语言:%3s\n", RunID, UserName, Problem, Lang);

	MySQL_SetOJAllRun();

	char Name_Copy[100];
	printf("评测机文件路径：%s\n", FileDP);
	char *FileName = strrchr(FileDP, '\\') + 1;
	strcpy_s(Name_Copy, FileName);
	int FileLen = (int)strlen(Name_Copy);
	Name_Copy[FileLen - 4] = '\0';
	printf("评测机：%s\n\n", Name_Copy);
	MySQL_SetJudgerName(RunID, Name_Copy);

	int proNum = atoi(Problem);

	//对程序进行评测
	JudgeSystem.Reset();
	JudgeSystem.SetJudgeMode(JudgeType);
	JudgeSystem.SetRunID(RunID);
	JudgeSystem.SetProblemNum(proNum);
	JudgeSystem.SetLanguage(Lang);
	JudgeSystem.SetTimeLimit(MaxTime);
	JudgeSystem.SetMemoryLimit(MaxMemory);

	if (JudgeSystem.Compile())
	{
		MySQL_ChangeStatus(RunID, Running);

		char AllTest[100];
		strcpy_s(AllTest, Test);

		char *buf[100] = {};
		char *str = strtok_s(AllTest, "&", buf);

		int allTest[100];
		int testNum = 1;

		while (str)
		{
			int i = atoi(str);

			allTest[testNum++] = i;

			str = strtok_s(buf[0], "&", buf);
		}

		JudgeSystem.SetTestNum(testNum - 1);
		JudgeSystem.SetAllTestNum(allTest);
		JudgeSystem.Run();
	}
	else
	{
		MySQL_ChangeStatus(RunID, CompileError);
	}
	
	Sleep(10);
}

//获取数据文件中的信息
void FileToData(const char *FileData, int &JudgeType, char *Problem, int &MaxTime, int &MaxMemory, char *UserName, char *Lang, char *Test)
{
	char iData[200];
	strcpy_s(iData, FileData);

	char *Context[1000] = {};
	//语言
	char *Data = strtok_s(iData, "|", Context);
	strcpy_s(Lang, 100, Data);
	//用户名
	Data = strtok_s(Context[0], "|", Context);
	strcpy_s(UserName, 100, Data);
	//题号
	Data = strtok_s(Context[0], "|", Context);
	strcpy_s(Problem, 100, Data);
	//评测模式
	Data = strtok_s(Context[0], "|", Context);
	JudgeType = atoi(Data);
	//最大时间
	Data = strtok_s(Context[0], "|", Context);
	MaxTime = atoi(Data);
	//最大内存
	Data = strtok_s(Context[0], "|", Context);
	MaxMemory = atoi(Data);
	//测试点
	Data = strtok_s(Context[0], "|", Context);
	strcpy_s(Test, 1000, Data);
}


void getFiles(const char *path, priority_queue<FileData_t> &files)
{
	intptr_t hFile = 0;
	_finddata_t fileinfo;
	string p;

	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if (!(fileinfo.attrib & _A_SUBDIR))
			{
				FileData_t item;
				item.RunID = atoi(fileinfo.name);

				char FilePath[1000];
				sprintf_s(FilePath, "%s\\%s", path, fileinfo.name);

				char buf[1000];
				ifstream is(FilePath);
				is.getline(buf, 1000);
				is.close();
				remove(FilePath);

				wchar_t *wBuf = UTF8ToUnicode(buf);
				char *AnsiBuf = UnicodeToANSI(wBuf);
				strcpy_s(item.Data, AnsiBuf);

				files.push(item);

				MySQL_ChangeStatus(item.RunID, Pending);
			}

		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);
	}
}

BOOL WINAPI CloseJudge(DWORD dwCtrlType)
{
	MySQL_SetOJState(0);
	MySQL_Delete();

	JudgeSystem.JudgerStop();
	return TRUE;
}

int main(int argc, char *argv[])
{
	MySQL_Connect();
	MySQL_SetOJState(1);

	FileDP = argv[0];

	JudgeSystem.SetNumofThread(5);
	JudgeSystem.SetNumofTimeLimit(1);
	JudgeSystem.SetRemoveBlank(true);
	JudgeSystem.SetJudgeMode(Judge_ErrorStop);

	char filePath[100];
#ifndef JUDGE_CONTEST
	strcpy_s(filePath, "Temporary_Data");
#endif
#ifdef JUDGE_CONTEST
	strcpy_s(filePath, "Temporary_ContestData");
#endif

	printf("%s ： 开始评测\n", MySQL_GetOJName());

	SetConsoleCtrlHandler(CloseJudge, TRUE);

	static priority_queue<FileData_t> coData;

	while (true)
	{
		getFiles(filePath, coData);

		while (coData.size() != 0)
		{
			//Judge
			int RunID = coData.top().RunID;

			char LogPath[100];
			sprintf_s(LogPath, "log\\Judge_%d.log", RunID);
			FILE *stream;
			freopen_s(&stream, LogPath, "w", stdout);

			fprintf(stdout, "%s\n", coData.top().Data);

			//问题
			char Problem[100];
			//用户名
			char UserName[100];
			//语言
			char Lang[100];
			//测试点
			char Test[1000];
			//最大时间
			int MaxTime;
			//最大内存
			int MaxMemory;
			//评测模式
			int JudgeType;

			FileToData(coData.top().Data, JudgeType, Problem, MaxTime, MaxMemory, UserName, Lang, Test);

			JudgeSystem.CreateDir(RunID);

			char Path1[100], Path2[100];
			sprintf_s(Path1, "Temporary_Code\\%d", RunID);
			
			if (!strcmp(Lang, "Java"))
			{
				sprintf_s(Path2, "test\\%d\\Main.java", RunID);
			}
			else if (!strcmp(Lang, "Python"))
			{
				sprintf_s(Path2, "test\\%d\\Code.py", RunID);
			}
			else if (!strcmp(Lang, "Gcc"))
			{
				sprintf_s(Path2, "test\\%d\\Code.c", RunID);
			}
			else
			{
				sprintf_s(Path2, "test\\%d\\Code.cpp", RunID);
			}

			CopyFile(Path1, Path2, TRUE);

			Judge(JudgeSystem, RunID, JudgeType, UserName, Problem, MaxTime, MaxMemory, Lang, Test);

			JudgeSystem.DeleteTestFile();

			coData.pop();
			fclose(stream);
		}

		Sleep(1000);
	}

	MySQL_SetOJState(0);
	MySQL_Delete();

	system("pause");
	return 0;
}