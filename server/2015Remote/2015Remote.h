
// 2015Remote.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������
#include "iniFile.h"

// CMy2015RemoteApp:
// �йش����ʵ�֣������ 2015Remote.cpp
//

class CMy2015RemoteApp : public CWinApp
{
public:
	CMy2015RemoteApp();
	iniFile  m_iniFile;
	HANDLE m_Mutex;
// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CMy2015RemoteApp theApp;