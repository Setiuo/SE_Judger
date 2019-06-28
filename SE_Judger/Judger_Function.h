#pragma once
#include <windows.h>

#define PATHLEN 200

//�̴߳��ݲ����ṹ��
struct JudgeDate
{
	//����ID
	int runID;
	//���
	int problemNum;
	//���Ե��
	int testNum;
	//����ڴ�ʹ������
	int memoryLimit;
	//���ʱ��ʹ������
	int timeLimit;
	bool resurvey;
};

//����ģʽ
//Judge_ErrorStop @��������ֹͣ����
//Judge_All		  @ȫ�����Ե�����
enum JudgeMode
{
	Judge_ErrorStop = 1,
	Judge_All
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
	"Null",
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
	//����ID
	int runID;
	//���
	int problemNum;
	//���Ե�����
	int testNumber;
	//ʱ������
	int timeLimit;
	//�ڴ�����
	int memoryLimit;
	//ÿ�����Ե�š������±��Ӧ�Ĳ��Ե�š�
	int *allTestNum;
	//����ģʽ
	int judgeMode;

	//�������������״̬
	int LastStatus;
	//���������ʹ��ʱ��
	int LastTimeUsed;
	//���������ʹ���ڴ�
	int LastMemoryUsed;

public:
	funJudger_t();
	~funJudger_t();

public:
	//��������߳���
	void SetNumofThread(int num);
	//���ó�ʱ�ز����
	void SetNumofTimeLimit(int num);
	//��������ģʽ
	void SetJudgeMode(int mode);

	//��������ID
	void SetRunID(int num);
	//���ò��Ե�����
	void SetTestNum(int num);
	//�������
	void SetProblemNum(int num);
	//����ÿ�����Ե��
	void SetAllTestNum(int *allTest);
	//�����ڴ�����
	void SetMemoryLimit(int num);
	//����ʱ������
	void SetTimeLimit(int num);
	//���ó�������
	void SetLanguage(const char *Name);
	//������������ʱ������ĩ�ո�ͻ��з�
	void SetRemoveBlank(bool Remove);
	//����������
	void Reset();

	//ɾ�������������
	void DeleteTestFile(int RunID);

	bool Compile();
	int Run();
	void PrintResult();
private:
	//���������߳�
	void CreateTestThread(int start, int end);
	//���������̣߳�һ�����ڳ�ʱ�ز�
	void CreateTestThread(int num, bool resurvey = false);

	//ȡ������������
	void GetResult();

	//������������Ϣ
	void PrintErrorLog(int RunID);

	//�����̵߳��ú���
	static DWORD WINAPI JudgeTest(LPVOID lpParamter);

	//������ļ���������
	static bool AcceptedTest(const char* program, const char* tester);
	static bool PresentationErrorTest(const char* program, const char* tester);
	static bool OutputLimitExceededTest(const char* program, const char* tester);
};