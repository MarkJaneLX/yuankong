// ScreenSpy.h: interface for the CScreenSpy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCREENSPY_H__5F74528D_9ABD_404E_84D2_06C96A0615F4__INCLUDED_)
#define AFX_SCREENSPY_H__5F74528D_9ABD_404E_84D2_06C96A0615F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define ALGORITHM_DIFF 1
#define COPY_ALL 1	// ����ȫ����Ļ�����ֿ鿽����added by yuanyuanxiang 2019-1-7��
#include "CursorInfo.h"


class CScreenSpy  
{
private:
	BYTE             m_bAlgorithm;       // ��Ļ�����㷨
	ULONG            m_ulbiBitCount;     // ÿ����λ��

	ULONG            m_ulFullWidth;      // ��Ļ��
	ULONG            m_ulFullHeight;     //��Ļ��
	bool             m_bZoomed;          // ��Ļ������
	double           m_wZoom;            // ��Ļ�������ű�
	double           m_hZoom;            // ��Ļ�������ű�

	LPBITMAPINFO     m_BitmapInfor_Full; // BMP��Ϣ

	HWND             m_hDeskTopWnd;      //��ǰ�������Ĵ��ھ��
	HDC              m_hFullDC;          //Explorer.exe �Ĵ����豸DC

	HDC              m_hFullMemDC;
	HBITMAP	         m_BitmapHandle;
	PVOID            m_BitmapData_Full;

	HDC              m_hDiffMemDC;
	HBITMAP	         m_DiffBitmapHandle;
	PVOID            m_DiffBitmapData_Full;

	ULONG            m_RectBufferOffset; // ������λ��
	BYTE*            m_RectBuffer;       // ������

public:
	CScreenSpy(ULONG ulbiBitCount);

	virtual ~CScreenSpy();

	int GetWidth()const {
		return m_ulFullWidth;
	}

	int GetHeight() const {
		return m_ulFullHeight;
	}

	bool IsZoomed() const {
		return m_bZoomed;
	}

	double GetWZoom() const {
		return m_wZoom;
	}

	double GetHZoom() const {
		return m_hZoom;
	}

	const LPBITMAPINFO& GetBIData() const
	{
		return m_BitmapInfor_Full;
	}

	ULONG GetFirstScreenLength() const
	{
		return m_BitmapInfor_Full->bmiHeader.biSizeImage;
	}

	FORCEINLINE VOID WriteRectBuffer(LPBYTE szBuffer, ULONG ulLength)
	{
		memcpy(m_RectBuffer + m_RectBufferOffset, szBuffer, ulLength);
		m_RectBufferOffset += ulLength;
	}

	LPVOID GetFirstScreenData();

	LPVOID GetNextScreenData(ULONG* ulNextSendLength);

	ULONG CompareBitmap(LPBYTE CompareSourData, LPBYTE CompareDestData, LPBYTE szBuffer, DWORD ulCompareLength);

	VOID ScanScreen(HDC hdcDest, HDC hdcSour, ULONG ulWidth, ULONG ulHeight);

};

#endif // !defined(AFX_SCREENSPY_H__5F74528D_9ABD_404E_84D2_06C96A0615F4__INCLUDED_)
