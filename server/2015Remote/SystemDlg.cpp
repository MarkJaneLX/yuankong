// SystemDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "2015Remote.h"
#include "SystemDlg.h"
#include "afxdialogex.h"


// CSystemDlg �Ի���

enum                  
{
	COMMAND_WINDOW_CLOSE,   //�رմ���
	COMMAND_WINDOW_TEST,    //��������
};
IMPLEMENT_DYNAMIC(CSystemDlg, CDialog)

	CSystemDlg::CSystemDlg(CWnd* pParent, IOCPServer* IOCPServer, CONTEXT_OBJECT *ContextObject)
	: CDialog(CSystemDlg::IDD, pParent)
{
	m_ContextObject = ContextObject;
	m_iocpServer = IOCPServer;

	char	*lpBuffer = (char *)(m_ContextObject->InDeCompressedBuffer.GetBuffer(0));  //���ض˴��ص�����
	m_bHow=lpBuffer[0];   
}

CSystemDlg::~CSystemDlg()
{
}

void CSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SYSTEM, m_ControlList);
}


BEGIN_MESSAGE_MAP(CSystemDlg, CDialog)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SYSTEM, &CSystemDlg::OnNMRClickListSystem)
	ON_COMMAND(ID_PLIST_KILL, &CSystemDlg::OnPlistKill)
	ON_COMMAND(ID_PLIST_REFRESH, &CSystemDlg::OnPlistRefresh)
	ON_COMMAND(ID_WLIST_REFRESH, &CSystemDlg::OnWlistRefresh)
	ON_COMMAND(ID_WLIST_CLOSE, &CSystemDlg::OnWlistClose)
	ON_COMMAND(ID_WLIST_HIDE, &CSystemDlg::OnWlistHide)
	ON_COMMAND(ID_WLIST_RECOVER, &CSystemDlg::OnWlistRecover)
	ON_COMMAND(ID_WLIST_MAX, &CSystemDlg::OnWlistMax)
	ON_COMMAND(ID_WLIST_MIN, &CSystemDlg::OnWlistMin)
END_MESSAGE_MAP()


// CSystemDlg ��Ϣ�������


BOOL CSystemDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString str;
	sockaddr_in  ClientAddr;
	memset(&ClientAddr, 0, sizeof(ClientAddr));
	int iSockAddrLength = sizeof(ClientAddr);
	BOOL bResult = getpeername(m_ContextObject->sClientSocket, (SOCKADDR*)&ClientAddr, &iSockAddrLength);
	m_bHow==TOKEN_PSLIST 
		? str.Format("%s - ���̹���", bResult != INVALID_SOCKET ? inet_ntoa(ClientAddr.sin_addr) : "")
		:str.Format("%s - ���ڹ���", bResult != INVALID_SOCKET ? inet_ntoa(ClientAddr.sin_addr) : "");
	SetWindowText(str);//���öԻ������

	m_ControlList.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT); 
	if (m_bHow==TOKEN_PSLIST)      //���̹����ʼ���б�
	{
		m_ControlList.InsertColumn(0, "ӳ������", LVCFMT_LEFT, 180);
		m_ControlList.InsertColumn(1, "PID", LVCFMT_LEFT, 70);
		m_ControlList.InsertColumn(2, "����·��", LVCFMT_LEFT, 320);
		ShowProcessList();   //���ڵ�һ������������Ϣ��������Ž��̵��������԰�������ʾ���б���\0\0
	}else if (m_bHow==TOKEN_WSLIST)//���ڹ����ʼ���б�
	{
		//��ʼ�� ���ڹ�����б�
		m_ControlList.InsertColumn(0, "���", LVCFMT_LEFT, 80);
		m_ControlList.InsertColumn(1, "��������", LVCFMT_LEFT, 420);
		m_ControlList.InsertColumn(2, "����״̬", LVCFMT_LEFT, 200);
		ShowWindowsList();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

void CSystemDlg::ShowWindowsList(void)
{
	char *szBuffer = (char *)(m_ContextObject->InDeCompressedBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	char	*szTitle = NULL;
	bool isDel=false;

	m_ControlList.DeleteAllItems();
	CString	str;
	int i ;
	for ( i = 0; dwOffset <m_ContextObject->InDeCompressedBuffer.GetBufferLength() - 1; ++i)
	{
		LPDWORD	lpPID = LPDWORD(szBuffer + dwOffset);   //���ھ��
		szTitle = (char *)szBuffer + dwOffset + sizeof(DWORD);   //���ڱ���    
		str.Format("%5u", *lpPID);
		m_ControlList.InsertItem(i, str);
		m_ControlList.SetItemText(i, 1, szTitle);
		m_ControlList.SetItemText(i, 2, "��ʾ"); //(d) ������״̬��ʾΪ "��ʾ"
		// ItemData Ϊ���ھ��
		m_ControlList.SetItemData(i, *lpPID);  //(d)   
		dwOffset += sizeof(DWORD) + lstrlen(szTitle) + 1;
	}
	str.Format("��������    ���ڸ�����%d��", i);   //�޸�CtrlList 
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_ControlList.SetColumn(1, &lvc);
}

void CSystemDlg::ShowProcessList(void)
{
	char	*szBuffer = (char *)(m_ContextObject->InDeCompressedBuffer.GetBuffer(1)); //xiaoxi[][][][][]
	char	*szExeFile;
	char	*szProcessFullPath;
	DWORD	dwOffset = 0;
	CString str;
	m_ControlList.DeleteAllItems();       
	//������������ÿһ���ַ��������������ݽṹ�� Id+������+0+������+0
	int i;
	for (i = 0; dwOffset < m_ContextObject->InDeCompressedBuffer.GetBufferLength() - 1; ++i)
	{
		LPDWORD	PID = LPDWORD(szBuffer + dwOffset);        //����õ�����ID
		szExeFile = szBuffer + dwOffset + sizeof(DWORD);      //����������ID֮�����
		szProcessFullPath = szExeFile + lstrlen(szExeFile) + 1;  //���������ǽ�����֮�����
		//�������ݽṹ�Ĺ���������

		m_ControlList.InsertItem(i, szExeFile);       //���õ������ݼ��뵽�б���
		str.Format("%5u", *PID);
		m_ControlList.SetItemText(i, 1, str);
		m_ControlList.SetItemText(i, 2, szProcessFullPath);
		// ItemData Ϊ����ID
		m_ControlList.SetItemData(i, *PID);

		dwOffset += sizeof(DWORD) + lstrlen(szExeFile) + lstrlen(szProcessFullPath) + 2;   //����������ݽṹ ������һ��ѭ��
	}

	str.Format("������� / %d", i); 
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_ControlList.SetColumn(2, &lvc); //���б�����ʾ�ж��ٸ�����
}


void CSystemDlg::OnClose()
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


void CSystemDlg::OnNMRClickListSystem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CMenu	Menu;
	if (m_bHow==TOKEN_PSLIST)      //���̹����ʼ���б�
	{
		Menu.LoadMenu(IDR_PROCESS_LIST);
	}else if (m_bHow==TOKEN_WSLIST)
	{
		Menu.LoadMenu(IDR_WINDOW_LIST);
	}
	CMenu*	SubMenu = Menu.GetSubMenu(0);
	CPoint	Point;
	GetCursorPos(&Point);
	SubMenu->TrackPopupMenu(TPM_LEFTALIGN, Point.x, Point.y, this);

	*pResult = 0;
}

void CSystemDlg::OnPlistKill()
{
	CListCtrl	*ListCtrl = NULL;
	if (m_ControlList.IsWindowVisible())
		ListCtrl = &m_ControlList;
	else
		return;

	//[KILL][ID][ID][iD][ID]
	//���仺����
	LPBYTE szBuffer = (LPBYTE)LocalAlloc(LPTR, 1 + (ListCtrl->GetSelectedCount() * 4));//1.exe  4  ID   Handle
	//����������̵�����ͷ
	szBuffer[0] = COMMAND_KILLPROCESS; 
	//��ʾ������Ϣ
	char *szTips = "����: ��ֹ���̻ᵼ�²�ϣ�������Ľ����\n"
		"�������ݶ�ʧ��ϵͳ���ȶ����ڱ���ֹǰ��\n"
		"���̽�û�л��ᱣ����״̬�����ݡ�";
	CString str;
	if (ListCtrl->GetSelectedCount() > 1)
	{
		str.Format("%sȷʵ\n����ֹ��%d�������?", szTips, ListCtrl->GetSelectedCount());	
	}
	else
	{
		str.Format("%sȷʵ\n����ֹ���������?", szTips);
	}
	if (::MessageBox(m_hWnd, str, "���̽�������", MB_YESNO|MB_ICONQUESTION) == IDNO)
		return;

	DWORD	dwOffset = 1;
	POSITION Pos = ListCtrl->GetFirstSelectedItemPosition(); 
	//�õ�Ҫ�����ĸ�����
	while(Pos) 
	{
		int	nItem = ListCtrl->GetNextSelectedItem(Pos);
		DWORD dwProcessID = ListCtrl->GetItemData(nItem);
		memcpy(szBuffer + dwOffset, &dwProcessID, sizeof(DWORD));  //sdkfj101112
		dwOffset += sizeof(DWORD);
	}
	//�������ݵ����ض��ڱ��ض��в���COMMAND_KILLPROCESS�������ͷ
	m_iocpServer->OnClientPreSending(m_ContextObject, szBuffer, LocalSize(szBuffer));
	LocalFree(szBuffer);

	Sleep(100);

	OnPlistRefresh();
}


VOID CSystemDlg::OnPlistRefresh()
{
	if (m_ControlList.IsWindowVisible())
	{
		m_ControlList.DeleteAllItems();
		GetProcessList();
		ShowProcessList();
	}
}


VOID CSystemDlg::GetProcessList(void)
{
	BYTE bToken = COMMAND_PSLIST;
	m_iocpServer->OnClientPreSending(m_ContextObject, &bToken, 1);
}


void CSystemDlg::OnWlistRefresh()
{
	GetWindowsList();
}


void CSystemDlg::GetWindowsList(void)
{
	BYTE bToken = COMMAND_WSLIST;
	m_iocpServer->OnClientPreSending(m_ContextObject, &bToken, 1);
}


void CSystemDlg::OnReceiveComplete(void)
{
	switch (m_ContextObject->InDeCompressedBuffer.GetBuffer(0)[0])
	{
	case TOKEN_PSLIST:
		{
			ShowProcessList();

			break;
		}
	case TOKEN_WSLIST:
		{
			ShowWindowsList();
			break;
		}

	default:
		// ���䷢���쳣����
		break;
	}
}


void CSystemDlg::OnWlistClose()
{
	BYTE lpMsgBuf[20];
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_ControlList;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem>=0)
	{

		ZeroMemory(lpMsgBuf,20);
		lpMsgBuf[0]=COMMAND_WINDOW_CLOSE;           //ע������������ǵ�����ͷ

		DWORD hwnd = pListCtrl->GetItemData(nItem); //�õ����ڵľ��һͬ����  4   djfkdfj  dkfjf  4
		memcpy(lpMsgBuf+1,&hwnd,sizeof(DWORD));   //1 4
		m_iocpServer->OnClientPreSending(m_ContextObject, lpMsgBuf, sizeof(lpMsgBuf));			

	}
}


void CSystemDlg::OnWlistHide()
{
	BYTE lpMsgBuf[20];
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_ControlList;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem>=0)
	{
		ZeroMemory(lpMsgBuf,20);
		lpMsgBuf[0]=COMMAND_WINDOW_TEST;             //���ڴ�������ͷ
		DWORD hwnd = pListCtrl->GetItemData(nItem);  //�õ����ڵľ��һͬ����
		pListCtrl->SetItemText(nItem,2,"����");      //ע����ʱ���б��е���ʾ״̬Ϊ"����"
		//������ɾ���б���Ŀʱ�Ͳ�ɾ�������� ���ɾ������ھ���ᶪʧ ����ԶҲ������ʾ��
		memcpy(lpMsgBuf+1,&hwnd,sizeof(DWORD));      //�õ����ڵľ��һͬ����
		DWORD dHow=SW_HIDE;                          //���ڴ������ 0
		memcpy(lpMsgBuf+1+sizeof(hwnd),&dHow,sizeof(DWORD));
		m_iocpServer->OnClientPreSending(m_ContextObject, lpMsgBuf, sizeof(lpMsgBuf));	
	}
}


void CSystemDlg::OnWlistRecover()
{
	BYTE lpMsgBuf[20];
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_ControlList;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem>=0)
	{
		ZeroMemory(lpMsgBuf,20);
		lpMsgBuf[0]=COMMAND_WINDOW_TEST;
		DWORD hwnd = pListCtrl->GetItemData(nItem);
		pListCtrl->SetItemText(nItem,2,"��ʾ");
		memcpy(lpMsgBuf+1,&hwnd,sizeof(DWORD));
		DWORD dHow=SW_NORMAL;
		memcpy(lpMsgBuf+1+sizeof(hwnd),&dHow,sizeof(DWORD));
		m_iocpServer->OnClientPreSending(m_ContextObject, lpMsgBuf, sizeof(lpMsgBuf));	
	}
}


void CSystemDlg::OnWlistMax()
{
	BYTE lpMsgBuf[20];
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_ControlList;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem>=0)
	{
		ZeroMemory(lpMsgBuf,20);
		lpMsgBuf[0]=COMMAND_WINDOW_TEST;
		DWORD hwnd = pListCtrl->GetItemData(nItem);
		pListCtrl->SetItemText(nItem,2,"��ʾ");
		memcpy(lpMsgBuf+1,&hwnd,sizeof(DWORD));
		DWORD dHow=SW_MAXIMIZE;
		memcpy(lpMsgBuf+1+sizeof(hwnd),&dHow,sizeof(DWORD));
		m_iocpServer->OnClientPreSending(m_ContextObject, lpMsgBuf, sizeof(lpMsgBuf));	
	}
}


void CSystemDlg::OnWlistMin()
{
	BYTE lpMsgBuf[20];
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_ControlList;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem>=0)
	{
		ZeroMemory(lpMsgBuf,20);
		lpMsgBuf[0]=COMMAND_WINDOW_TEST;
		DWORD hwnd = pListCtrl->GetItemData(nItem);
		pListCtrl->SetItemText(nItem,2,"��ʾ");
		memcpy(lpMsgBuf+1,&hwnd,sizeof(DWORD));
		DWORD dHow=SW_MINIMIZE;
		memcpy(lpMsgBuf+1+sizeof(hwnd),&dHow,sizeof(DWORD));
		m_iocpServer->OnClientPreSending(m_ContextObject, lpMsgBuf, sizeof(lpMsgBuf));	

	}
}
