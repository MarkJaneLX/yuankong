#include "stdafx.h"
#include "CpuUseage.h"

//ϵͳ���ܼ�����
CCpuUsage::CCpuUsage()
{
	m_hQuery = NULL;
	m_pCounterStruct = NULL;

}

CCpuUsage::~CCpuUsage()
{
	PdhCloseQuery(m_hQuery);   //�رռ�����
	if (m_pCounterStruct)
		delete m_pCounterStruct;
}


BOOL CCpuUsage::Init()
{
	if (ERROR_SUCCESS != PdhOpenQuery(NULL, 1, &m_hQuery))   //�򿪼�����
		return FALSE;

	m_pCounterStruct = (PPDHCOUNTERSTRUCT) new PDHCOUNTERSTRUCT;

	//ͳ�Ƹ���Ȥ��ϵͳ��Ϣʱ�������Ƚ���Ӧ�ļ�������ӽ���
	PDH_STATUS pdh_status = PdhAddCounter(m_hQuery, (LPCSTR)szCounterName, 
		(DWORD) m_pCounterStruct, &(m_pCounterStruct->hCounter));
	if (ERROR_SUCCESS != pdh_status) 
	{
		return FALSE;
	}

	return TRUE;
}


int CCpuUsage::GetUsage()
{
	PDH_FMT_COUNTERVALUE pdhFormattedValue;

	PdhCollectQueryData(m_hQuery);

	//�õ����ݻ�õ�ǰ��������ֵ
	if (ERROR_SUCCESS != PdhGetFormattedCounterValue(m_pCounterStruct->hCounter,
		PDH_FMT_LONG,
		NULL,
		&pdhFormattedValue )) 
	{
		return 0;
	}

	return pdhFormattedValue.longValue;
}
