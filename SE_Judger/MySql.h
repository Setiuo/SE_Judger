#pragma once

//�������ݿ�
extern void MySQL_Connect();

//��ȡOJ����
extern char *MySQL_GetOJName();

//�������������
extern void MySQL_SetJudgerName(int RunID, const char *Name);

//�������������״̬
extern void MySQL_SetOJState(int State);

//����������������
extern void MySQL_SetOJAllRun();

//��ȡ������������
extern int MySQL_GetOJAllRun();

//�������ݿ�������
extern void MySQL_ChangeStatus(int RunID, const int Status);

//�������ݿ����ÿ�����Ե������״̬������ʱ���ڴ��
extern void MySQL_SetAllStatus(int RunID, const char *Status);

//�������ݿ��������� ʹ�õ�ʱ��
extern void MySQL_SetUseTime(int RunID, int UseTime);

//�������ݿ��������� ʹ�õ��ڴ�
extern void MySQL_SetUseMemory(int RunID, int Memory);

//ע�����ݿ�
extern void MySQL_Delete();