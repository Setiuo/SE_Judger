#pragma once
#include <windows.h>

#define PATHLEN 200

//线程传递参数结构体
struct JudgeDate
{
	//运行ID
	int runID;
	//题号
	int problemNum;
	//测试点号
	int testNum;
	//最大内存使用限制
	int memoryLimit;
	//最大时间使用限制
	int timeLimit;
};

enum Language
{
	SE_C,
	SE_CPlusPlus,
	SE_Java,
	SE_Python
};

enum ProgramState
{
	Null,
	Wating,
	Pending,
	Compiling,
	Running,
	Accepted,
	PresentationError,
	TimeLimitExceeded,
	MemoryLimitExceeded,
	WrongAnswer,
	RuntimeError,
	OutputLimitExceeded,
	CompileError,
	SystemError
};

const char ProgramStateStr[][30]
{
	"",
	"Wating",
	"Pending",
	"Compiling",
	"Running",
	"Accepted",
	"Presentation Error",
	"Time Limit Exceeded",
	"Memory Limit Exceeded",
	"Wrong Answer",
	"Runtime Error",
	"Output Limit Exceeded",
	"Compile Error",
	"System Error"
};

class funJudger_t
{
private:
	//运行ID
	int runID;
	//题号
	int problemNum;
	//测试点数量
	int testNumber;
	//使用语言
	int language;
	//时间限制
	int timeLimit;
	//内存限制
	int memoryLimit;
	//每个测试点号
	int *allTestNum;

	//结果
	int LastStatus;
	int LastTimeUsed;
	int LastMemoryUsed;

public:
	funJudger_t();
	~funJudger_t();

public:
	//设置最大线程数
	void SetNumofThread(int num);

	//设置运行ID
	void SetRunID(int num);
	//设置测试点数量
	void SetTestNum(int num);
	//设置题号
	void SetProblemNum(int num);
	//设置每个测试点号
	void SetAllTestNum(int *allTest);
	//设置内存限制
	void SetMemoryLimit(int num);
	//设置时间限制
	void SetTimeLimit(int num);
	//设置程序语言
	void SetLanguage(const char *Name);
	//删除测试输出数据
	void DeleteTestFile(int RunID);

	bool Compile();
	int Run();
private:
	//创建评测线程
	void CreateTestThread(int start, int end);
	//取得最终评测结果
	void GetResult();
	//输出编译错误信息
	void PrintErrorLog(int RunID);

	//评测线程调用函数
	static DWORD WINAPI JudgeTest(LPVOID lpParamter);

	//对输出文件进行评测
	static bool AcceptedTest(const char* program, const char* tester);
	static bool PresentationErrorTest(const char* program, const char* tester);
	static bool OutputLimitExceededTest(const char* program, const char* tester);
};