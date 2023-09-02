// ShellDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "2015Remote.h"
#include "ShellDlg.h"
#include "afxdialogex.h"


// CShellDlg �Ի���

IMPLEMENT_DYNAMIC(CShellDlg, CDialog)

CShellDlg::CShellDlg(CWnd* pParent, IOCPServer* IOCPServer, CONTEXT_OBJECT *ContextObject)
	: CDialog(CShellDlg::IDD, pParent)
{
	m_iocpServer	= IOCPServer;
	m_ContextObject		= ContextObject;

	m_hIcon			= LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SHELL)); 
}

CShellDlg::~CShellDlg()
{
}

void CShellDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT, m_Edit);
}


BEGIN_MESSAGE_MAP(CShellDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CShellDlg ��Ϣ�������


BOOL CShellDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_nCurSel = 0;
	m_nReceiveLength = 0;
	SetIcon(m_hIcon,FALSE);

	CString str;
	sockaddr_in  ClientAddr;
	memset(&ClientAddr, 0, sizeof(ClientAddr));
	int ClientAddrLen = sizeof(ClientAddr);
	BOOL bResult = getpeername(m_ContextObject->sClientSocket, (SOCKADDR*)&ClientAddr, &ClientAddrLen);

	str.Format("%s - Զ���ն�", bResult != INVALID_SOCKET ? inet_ntoa(ClientAddr.sin_addr) : "");
	SetWindowText(str);

	BYTE bToken = COMMAND_NEXT;
	m_iocpServer->OnClientPreSending(m_ContextObject, &bToken, sizeof(BYTE));  

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}


VOID CShellDlg::OnReceiveComplete()
{
	if (m_ContextObject==NULL)
	{
		return;
	}

	AddKeyBoardData();
	m_nReceiveLength = m_Edit.GetWindowTextLength();
}


VOID CShellDlg::AddKeyBoardData(void)
{
	// �������0

	//Hello>dir
	//Shit\0
	m_ContextObject->InDeCompressedBuffer.WriteBuffer((LPBYTE)"", 1);           //�ӱ����ƶ�������������Ҫ����һ��\0
	CString strResult = (char*)m_ContextObject->InDeCompressedBuffer.GetBuffer(0);    //������е����� ���� \0

	//�滻��ԭ���Ļ��з�  ����cmd �Ļ���ͬw32�µı༭�ؼ��Ļ��з���һ��   ���еĻس�����   
	strResult.Replace("\n", "\r\n");

	//�õ���ǰ���ڵ��ַ�����
	int	iLength = m_Edit.GetWindowTextLength();    //kdfjdjfdir
	//hello                                    
	//1.txt
	//2.txt
	//dir\r\n

	//����궨λ����λ�ò�ѡ��ָ���������ַ�  Ҳ����ĩβ ��Ϊ�ӱ��ض��������� Ҫ��ʾ�� ���ǵ� ��ǰ���ݵĺ���
	m_Edit.SetSel(iLength, iLength);

	//�ô��ݹ����������滻����λ�õ��ַ�    //��ʾ
	m_Edit.ReplaceSel(strResult);   

	//���µõ��ַ��Ĵ�С

	m_nCurSel = m_Edit.GetWindowTextLength();   //Hello 

	//����ע�⵽��������ʹ��Զ���ն�ʱ �����͵�ÿһ�������� ����һ�����з�  ����һ���س�
	//Ҫ�ҵ�����س��Ĵ������Ǿ�Ҫ��PreTranslateMessage�����Ķ���  
}

void CShellDlg::OnClose()
{
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


BOOL CShellDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// ����VK_ESCAPE��VK_DELETE
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_DELETE)
			return true;
		//����ǿɱ༭��Ļس���
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_Edit.m_hWnd)
		{
			//�õ����ڵ����ݴ�С
			int	iLength = m_Edit.GetWindowTextLength();
			CString str;
			//�õ����ڵ��ַ�����
			m_Edit.GetWindowText(str);
			//���뻻�з�
			str += "\r\n";
			//�õ������Ļ��������׵�ַ�ټ���ԭ�е��ַ���λ�ã���ʵ�����û���ǰ�����������
			//Ȼ�����ݷ��ͳ�ȥ
			LPBYTE pSrc = (LPBYTE)str.GetBuffer(0) + m_nCurSel;
#ifdef _DEBUG
			OutputDebugStringA("[Shell]=> ");
			OutputDebugStringA((char*)pSrc);
#endif
			int length = str.GetLength() - m_nCurSel;
			m_iocpServer->OnClientPreSending(m_ContextObject, pSrc, length);
			m_nCurSel = m_Edit.GetWindowTextLength();
			if (0 == strcmp((char*)pSrc, "exit\r\n"))
			{
				ShowWindow(SW_HIDE);
			}
		}
		// ����VK_BACK
		if (pMsg->wParam == VK_BACK && pMsg->hwnd == m_Edit.m_hWnd)
		{
			if (m_Edit.GetWindowTextLength() <= m_nReceiveLength)  
				return true;
		}
		// ʾ����
		//dir\r\n  5
		//hello\r\n 7
	}

	return CDialog::PreTranslateMessage(pMsg);
}


HBRUSH CShellDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((pWnd->GetDlgCtrlID() == IDC_EDIT) && (nCtlColor == CTLCOLOR_EDIT))
	{
		COLORREF clr = RGB(255, 255, 255);
		pDC->SetTextColor(clr);   //���ð�ɫ���ı�
		clr = RGB(0,0,0);
		pDC->SetBkColor(clr);     //���ú�ɫ�ı���
		return CreateSolidBrush(clr);  //��ΪԼ�������ر���ɫ��Ӧ��ˢ�Ӿ��
	}
	else
	{
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	return hbr;
}
