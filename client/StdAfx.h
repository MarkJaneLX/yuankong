// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__46CA6496_AAD6_4658_B6E9_D7AEB26CDCD5__INCLUDED_)
#define AFX_STDAFX_H__46CA6496_AAD6_4658_B6E9_D7AEB26CDCD5__INCLUDED_

// ʹ��ѹ���㷨���㷨��Ҫ��server��stdafx.hƥ��
#define USING_COMPRESS 1

// �Ƿ�ʹ��ZLIB
#define USING_ZLIB 1

#if !USING_ZLIB
// �Ƿ�ʹ��LZ4
#define USING_LZ4 1
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ����ڴ�й©���谲װVLD��������ע�ʹ���
#include "vld.h"

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__46CA6496_AAD6_4658_B6E9_D7AEB26CDCD5__INCLUDED_)

#include <assert.h>
#include <MMSystem.h>
#pragma comment(lib, "winmm.lib")

// �߾��ȵ�˯�ߺ���
#define Sleep_m(ms) { timeBeginPeriod(1); Sleep(ms); timeEndPeriod(1); }

// �Բ���n����������C�µȴ�T��(n�ǲ���������������1000)
#define WAIT_n(C, T, n) {assert(!(1000%(n)));int s=(1000*(T))/(n);do{Sleep(n);}while((C)&&(--s));}

// ������C����ʱ�ȴ�T��(����10ms)
#define WAIT(C, T) { timeBeginPeriod(1); WAIT_n(C, T, 10); timeEndPeriod(1); }

// ������C����ʱ�ȴ�T��(����1ms)
#define WAIT_1(C, T) { timeBeginPeriod(1); WAIT_n(C, T, 1); timeEndPeriod(1); }

#include <time.h>
#include <stdio.h>

// ���ܼ�ʱ�������㺯���ĺ�ʱ
class auto_tick
{
private:
	const char *func;
	int span;
	clock_t tick;
	__inline clock_t now() const { return clock(); }
	__inline int time() const { return now() - tick; }

public:
	auto_tick(const char *func_name, int th = 5) : func(func_name), span(th), tick(now()) { }
	~auto_tick() { stop(); }

	__inline void stop() {
		if (span != 0) { int s(this->time()); if (s > span)printf("[%s]ִ��ʱ��: [%d]ms.\n", func, s); span = 0; }
	}
};

#ifdef _DEBUG
// ���ܼ��㵱ǰ�����ĺ�ʱ����ʱ���ӡ
#define AUTO_TICK(thresh) auto_tick TICK(__FUNCTION__, thresh)
#define STOP_TICK TICK.stop()
#else
#define AUTO_TICK(thresh) 
#define STOP_TICK
#endif
