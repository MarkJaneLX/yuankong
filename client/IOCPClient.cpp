// IOCPClient.cpp: implementation of the IOCPClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IOCPClient.h"
#include <IOSTREAM>
#if USING_ZLIB
#include "zlib.h"
#define Z_FAILED(p) (Z_OK != (p))
#define Z_SUCCESS(p) (!Z_FAILED(p))
#else
#if USING_LZ4
#include "lz4/lz4.h"
#pragma comment(lib, "lz4/lz4.lib")
#define Z_FAILED(p) (0 == (p))
#define Z_SUCCESS(p) (!Z_FAILED(p))
#define compress(dest, destLen, source, sourceLen) LZ4_compress_default((const char*)source, (char*)dest, sourceLen, *(destLen))
#define uncompress(dest, destLen, source, sourceLen) LZ4_decompress_safe((const char*)source, (char*)dest, sourceLen, *(destLen))
#else
#include "zstd/zstd.h"
#pragma comment(lib, "zstd/zstd.lib")
#define Z_FAILED(p) ZSTD_isError(p)
#define Z_SUCCESS(p) (!Z_FAILED(p))
#define compress(dest, destLen, source, sourceLen) ZSTD_compress(dest, *(destLen), source, sourceLen, ZSTD_CLEVEL_DEFAULT)
#define uncompress(dest, destLen, source, sourceLen) ZSTD_decompress(dest, *(destLen), source, sourceLen)
#endif
#endif
#include <assert.h>
#include "Manager.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VOID IOCPClient::setManagerCallBack(CManager* Manager)
{
	m_Manager = Manager;
}


IOCPClient::IOCPClient(bool exit_while_disconnect)
{
	m_Manager = NULL;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	m_sClientSocket = INVALID_SOCKET;
	m_hWorkThread   = NULL;
	m_bWorkThread = S_STOP;

	memset(m_szPacketFlag, 0, sizeof(m_szPacketFlag));
	memcpy(m_szPacketFlag,"Shine",FLAG_LENGTH);

	m_bIsRunning = TRUE;
	m_bConnected = FALSE;

	InitializeCriticalSection(&m_cs);
	m_exit_while_disconnect = exit_while_disconnect;
}

IOCPClient::~IOCPClient()
{
	m_bIsRunning = FALSE;

	if (m_sClientSocket!=INVALID_SOCKET)
	{
		closesocket(m_sClientSocket);
		m_sClientSocket = INVALID_SOCKET;
	}

	if (m_hWorkThread!=NULL)
	{
		CloseHandle(m_hWorkThread);
		m_hWorkThread = NULL;
	}

	WSACleanup();

	while (S_RUN == m_bWorkThread)
		Sleep(10);

	DeleteCriticalSection(&m_cs);

	m_bWorkThread = S_END;
}

// ��������ȡIP��ַ
inline string GetIPAddress(const char *hostName)
{
	struct hostent *host = gethostbyname(hostName);
#ifdef _DEBUG
	printf("��������IP����Ϊ: %s.\n", host->h_addrtype == AF_INET ? "IPV4" : "IPV6");
	for (int i = 0; host->h_addr_list[i]; ++i)
		printf("��ȡ�ĵ�%d��IP: %s\n", i+1, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
#endif
	return host ? inet_ntoa(*(struct in_addr*)host->h_addr_list[0]) : "";
}

BOOL IOCPClient::ConnectServer(char* szServerIP, unsigned short uPort)
{
	m_sClientSocket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);    //�����

	if (m_sClientSocket == SOCKET_ERROR)   
	{ 
		return FALSE;   
	}

	//����sockaddr_in�ṹ Ҳ�������ض˵Ľṹ
	sockaddr_in	ServerAddr;
	ServerAddr.sin_family	= AF_INET;               //�����  IP
	ServerAddr.sin_port	= htons(uPort);	
	// ��szServerIP�����ֿ�ͷ������Ϊ�������������IPת��
	string server = ('0' <= szServerIP[0] && szServerIP[0] <= '9') 
		? szServerIP : GetIPAddress(szServerIP);
	ServerAddr.sin_addr.S_un.S_addr = inet_addr(server.c_str());

	if (connect(m_sClientSocket,(SOCKADDR *)&ServerAddr,sizeof(sockaddr_in)) == SOCKET_ERROR) 
	{
		if (m_sClientSocket!=INVALID_SOCKET)
		{
			closesocket(m_sClientSocket);
			m_sClientSocket = INVALID_SOCKET;
		}
		return FALSE;
	}

	const int chOpt = 1; // True
	// Set KeepAlive �����������, ��ֹ����˲���������
	if (setsockopt(m_sClientSocket, SOL_SOCKET, SO_KEEPALIVE,
		(char *)&chOpt, sizeof(chOpt)) == 0)
	{
		// ���ó�ʱ��ϸ��Ϣ
		tcp_keepalive	klive;
		klive.onoff = 1; // ���ñ���
		klive.keepalivetime = 1000 * 60 * 3; // 3���ӳ�ʱ Keep Alive
		klive.keepaliveinterval = 1000 * 5;  // ���Լ��Ϊ5�� Resend if No-Reply
		WSAIoctl(m_sClientSocket, SIO_KEEPALIVE_VALS,&klive,sizeof(tcp_keepalive),
			NULL,	0,(unsigned long *)&chOpt,0,NULL);
	}
	if (m_hWorkThread == NULL){
		m_hWorkThread = (HANDLE)CreateThread(NULL, 0, 
			(LPTHREAD_START_ROUTINE)WorkThreadProc,(LPVOID)this, 0, NULL);
		m_bWorkThread = m_hWorkThread ? S_RUN : S_STOP;
	}
	std::cout<<"���ӷ���˳ɹ�.\n";
	m_bConnected = TRUE;
	return TRUE;
}

DWORD WINAPI IOCPClient::WorkThreadProc(LPVOID lParam)
{
	IOCPClient* This = (IOCPClient*)lParam;
	char szBuffer[MAX_RECV_BUFFER] = {0};
	fd_set fd;
	const struct timeval tm = { 2, 0 };

	while (This->IsRunning()) // û���˳�����һֱ�������ѭ����
	{
		if(!This->IsConnected())
		{
			Sleep(50);
			continue;
		}
		FD_ZERO(&fd);
		FD_SET(This->m_sClientSocket, &fd);
		int iRet = select(NULL, &fd, NULL, NULL, &tm);
		if (iRet <= 0)      
		{
			if (iRet == 0) Sleep(50);
			else
			{
				printf("[select] return %d, GetLastError= %d. \n", iRet, WSAGetLastError());
				This->Disconnect(); //���մ�����
				if(This->m_exit_while_disconnect)
					break;
			}
		}
		else if (iRet > 0)
		{
			memset(szBuffer, 0, sizeof(szBuffer));
			int iReceivedLength = recv(This->m_sClientSocket,
				szBuffer,sizeof(szBuffer), 0); //�������ض˷���������
			if (iReceivedLength <= 0)
			{
				int a = GetLastError();
				This->Disconnect(); //���մ�����
				if(This->m_exit_while_disconnect)
					break;
			}else{
				//��ȷ���վ͵���OnRead����,ת��OnRead
				This->OnServerReceiving(szBuffer, iReceivedLength);
				if (This->m_Manager!=NULL && This->m_Manager->m_bIsDead)
				{
					printf("****** Recv bye bye ******\n");
					// �����Ƿ��˳��ͻ��˺����ضˣ����˳��ͻ���
					extern BOOL g_bExit;
					g_bExit = This->m_Manager->m_bIsDead;
					break;
				}
			}
		}
	}
	This->m_bWorkThread = S_STOP;
	This->m_bIsRunning = FALSE;

	return 0xDEAD;
}


VOID IOCPClient::OnServerReceiving(char* szBuffer, ULONG ulLength)
{
	try
	{
		assert (ulLength > 0);	
		//���½ӵ����ݽ��н�ѹ��
		CBuffer m_CompressedBuffer;
		m_CompressedBuffer.WriteBuffer((LPBYTE)szBuffer, ulLength);

		//��������Ƿ��������ͷ��С ��������ǾͲ�����ȷ������
		while (m_CompressedBuffer.GetBufferLength() > HDR_LENGTH)
		{
			char szPacketFlag[FLAG_LENGTH + 3] = {0};
			CopyMemory(szPacketFlag, m_CompressedBuffer.GetBuffer(),FLAG_LENGTH);
			//�ж�����ͷ
			if (memcmp(m_szPacketFlag, szPacketFlag, FLAG_LENGTH) != 0)
			{
				throw "Bad Buffer";
			}

			ULONG ulPackTotalLength = 0;
			CopyMemory(&ulPackTotalLength, m_CompressedBuffer.GetBuffer(FLAG_LENGTH), 
				sizeof(ULONG));

			//--- ���ݵĴ�С��ȷ�ж�
			if (ulPackTotalLength && 
				(m_CompressedBuffer.GetBufferLength()) >= ulPackTotalLength)
			{
				m_CompressedBuffer.ReadBuffer((PBYTE)szPacketFlag, FLAG_LENGTH);//��ȡ����ͷ�� shine

				m_CompressedBuffer.ReadBuffer((PBYTE) &ulPackTotalLength, sizeof(ULONG));

				ULONG ulOriginalLength = 0;
				m_CompressedBuffer.ReadBuffer((PBYTE) &ulOriginalLength, sizeof(ULONG));

				//50  
				ULONG ulCompressedLength = ulPackTotalLength - HDR_LENGTH;
				PBYTE CompressedBuffer = new BYTE[ulCompressedLength];
				PBYTE DeCompressedBuffer = new BYTE[ulOriginalLength];

				m_CompressedBuffer.ReadBuffer(CompressedBuffer, ulCompressedLength);

				int	iRet = uncompress(DeCompressedBuffer, 
					&ulOriginalLength, CompressedBuffer, ulCompressedLength);

				if (Z_SUCCESS(iRet))//�����ѹ�ɹ�
				{
					CBuffer m_DeCompressedBuffer;
					m_DeCompressedBuffer.WriteBuffer(DeCompressedBuffer,
						ulOriginalLength);

					//��ѹ�õ����ݺͳ��ȴ��ݸ�����Manager���д��� ע�����������˶�̬
					//����m_pManager�е����಻һ����ɵ��õ�OnReceive������һ��
					m_Manager->OnReceive((PBYTE)m_DeCompressedBuffer.GetBuffer(0),
						m_DeCompressedBuffer.GetBufferLength());
				}
				else{
					printf("[ERROR] uncompress failed \n");
					delete [] CompressedBuffer;
					delete [] DeCompressedBuffer;
					throw "Bad Buffer";
				}

				delete [] CompressedBuffer;
				delete [] DeCompressedBuffer;
			}
			else
				break;
		}
	}catch(...) { 
		printf("[ERROR] OnServerReceiving catch an error \n");
	}
}


// ��server�������ݣ�ѹ�������ȽϺ�ʱ��
// �ر�ѹ������ʱ��SendWithSplit�ȽϺ�ʱ��
BOOL IOCPClient::OnServerSending(const char* szBuffer, ULONG ulOriginalLength)  //Hello
{
	AUTO_TICK(50);
	assert (ulOriginalLength > 0);
	{
		//����1.001�������Ҳ��������ѹ����ռ�õ��ڴ�ռ��ԭ��һ�� +12
		//��ֹ���������//  HelloWorld  10   22
		//����ѹ�� ѹ���㷨 ΢���ṩ
		//nSize   = 436
		//destLen = 448
#if USING_COMPRESS
#if USING_ZLIB
		unsigned long	ulCompressedLength = (double)ulOriginalLength * 1.001  + 12;
#elif USING_LZ4
		unsigned long	ulCompressedLength = LZ4_compressBound(ulOriginalLength);
#else
		unsigned long	ulCompressedLength = ZSTD_compressBound(ulOriginalLength);
#endif
		LPBYTE			CompressedBuffer = new BYTE[ulCompressedLength];

		int	iRet = compress(CompressedBuffer, &ulCompressedLength, (PBYTE)szBuffer, ulOriginalLength);
		if (Z_FAILED(iRet))
		{
			printf("[ERROR] compress failed \n");
			delete [] CompressedBuffer;
			return FALSE;
		}
#if !USING_ZLIB
		ulCompressedLength = iRet;
#endif

#else // ��ѹ��
		unsigned long ulCompressedLength = ulOriginalLength;
		LPBYTE CompressedBuffer = (LPBYTE)szBuffer;
#endif
		ULONG ulPackTotalLength = ulCompressedLength + HDR_LENGTH;
		CBuffer m_WriteBuffer;

		m_WriteBuffer.WriteBuffer((PBYTE)m_szPacketFlag, FLAG_LENGTH);

		m_WriteBuffer.WriteBuffer((PBYTE) &ulPackTotalLength,sizeof(ULONG));
		//  5      4
		//[Shine][ 30 ]
		m_WriteBuffer.WriteBuffer((PBYTE)&ulOriginalLength, sizeof(ULONG));
		//  5      4    4
		//[Shine][ 30 ][5]
		m_WriteBuffer.WriteBuffer(CompressedBuffer,ulCompressedLength);

#if USING_COMPRESS // ʹ��ѹ���㷨����Ҫ�ͷ�������ڴ�
		delete [] CompressedBuffer;
		CompressedBuffer = NULL;
#endif

		// �ֿ鷢��
		//shine[0035][0010][HelloWorld+12]
		return SendWithSplit((char*)m_WriteBuffer.GetBuffer(), m_WriteBuffer.GetBufferLength(), 
			MAX_SEND_BUFFER);
	}
}

//  5    2   //  2  2  1
BOOL IOCPClient::SendWithSplit(const char* szBuffer, ULONG ulLength, ULONG ulSplitLength)
{
	AUTO_TICK(25);
	int			 iReturn = 0;   //���������˶���
	const char*  Travel = szBuffer;
	int			 i = 0;
	int			 ulSended = 0;
	const int	 ulSendRetry = 15;
	// ���η���
	for (i = ulLength; i >= ulSplitLength; i -= ulSplitLength)
	{
		int j = 0;
		for (; j < ulSendRetry; ++j)
		{
			iReturn = send(m_sClientSocket, Travel, ulSplitLength, 0);
			if (iReturn > 0)
			{
				break;
			}
		}
		if (j == ulSendRetry)
		{
			return FALSE;
		}

		ulSended += iReturn;
		Travel += ulSplitLength;
	}
	// �������Ĳ���
	if (i>0)  //1024
	{
		int j = 0;
		for (; j < ulSendRetry; j++)
		{
			iReturn = send(m_sClientSocket, (char*)Travel,i,0);

			if (iReturn > 0)
			{
				break;
			}
		}
		if (j == ulSendRetry)
		{
			return FALSE;
		}
		ulSended += iReturn;
	}

	return (ulSended == ulLength) ? TRUE : FALSE;
}


VOID IOCPClient::Disconnect() 
{
	std::cout<<"�Ͽ��ͷ���˵�����.\n";

	CancelIo((HANDLE)m_sClientSocket);
	closesocket(m_sClientSocket);	
	m_sClientSocket = INVALID_SOCKET;

	m_bConnected = FALSE;
}


VOID IOCPClient::RunEventLoop(const BOOL &bCondition)
{
	OutputDebugStringA("======> RunEventLoop begin\n");
	while (m_bIsRunning && bCondition)
		Sleep(200);
	OutputDebugStringA("======> RunEventLoop end\n");
}
