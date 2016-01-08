#include "stdafx.h"
#include <winsock2.h> 
#include <cstring>
#include <iostream>

#include "MD5.h"
#include "ras.h"
#include "rdial.h"

#pragma comment(lib, "ws2_32") 
#pragma comment(lib, "RASAPI32.LIB")


Rdial::Rdial(CString username, INT ver, long lasttimec)
:m_username(username),RADIUS("cqxinliradius002"),LR("\r\n")
{
	m_ver = ver;
	m_lasttimec = lasttimec;
}


CString Rdial::Realusername()
{
	time_t m_time = 0;						//�õ�ϵͳʱ�䣬��1970.01.01.00:00:00 ��ʼ������
	long m_time1c = 0;						//ʱ�������m_time1cΪ���,����ʱ�������ĵ�һ�μ���
	long temp = 0;
	int i = 0, j = 0, k = 0;
	unsigned int lenth = 0;

	unsigned char ss[4] = {0};		//Դ����1,��m_time1convert���м���õ���ʽ��Դ����
	unsigned char pad1[4] = {0};

	//��ʽ��������
	unsigned char pp[4] = {0};
	unsigned char pf[6] = {0};
	char temp1[100];

	CString strS1;						//md5���ܲ�����һ����,ss2��������ʽ
	CString strInput;
	CString m_formatsring;				//��m_timece������ַ���,һ��Ϊ�����ַ�
	CString m_md5;						//�Գ�����(m_timec�ַ�����ʾ+m_username+radius)��MD5����
	CString m_md5use;					//md5 Lowerģʽ��ǰ��λ


	//ȡ��ϵͳʱ��m_time
	time(&m_time);
	//ʱ�������m_time1cΪ���,����ʱ�������ĵ�һ�μ���
	//�Ӻ���////////////////////////////

	m_time1c = (m_time * 0x66666667) >> 0x21;

	//5���ڶ�̬�û���һ�´���
	if (m_time1c <= m_lasttimec)
	{
		m_time1c = m_lasttimec + 1;
	}
	m_lasttimec = m_time1c;

	temp = htonl(m_time1c);
	memcpy(pad1, &temp, 4);

	for (int i = 0; i < 4; i++)
	{
		strS1 += pad1[i];
	}

	memcpy(ss, &m_time1c, 4);

	//�Ӻ���////////////////////////////

	for (i = 0; i < 32; i++)
	{
		j = i / 8;
		k = 3 - (i % 4);
		pp[k] *= 2;
		if (ss[j] % 2 == 1)
		{
			pp[k]++;
		}
		ss[j] /= 2;
	}
	

	pf[0] = pp[3] / 0x4;
	pf[1] = (pp[2] / 0x10) | ((pp[3] & 0x3) * 0x10);
	pf[2] = (pp[1] / 0x40) | (pp[2] & 0x0F) * 0x04;
	pf[3] = pp[1] & 0x3F;
	pf[4] = pp[0] / 0x04;
	pf[5] = (pp[0] & 0x03) * 0x10;

	/////////////////////////////////////

	for (i = 0; i < 6; i++)
	{
		pf[i] += 0x20;
		if ((pf[i]) >= 0x40)
		{
			pf[i]++;
		}
	}
	 
	for (i = 0; i < 6; i++)
	{
		m_formatsring += pf[i];
	}
	
	/////////////////////////////////////

	strInput = strS1 + m_username.Left(m_username.FindOneOf("@")) + RADIUS;
	lenth = 20 + m_username.FindOneOf("@");
	memcpy(temp1, strInput.GetBuffer(100), 100);
	m_md5 = MD5String(temp1, lenth);
	m_md5use = m_md5.Left(2);
	m_realusername = LR + m_formatsring + m_md5use + m_username;

// #define _debug	
// #ifdef _debug
// cout<<"m_username.FindOneOf(\"@\"):"<<m_username.FindOneOf("@")<<"\nm_username.left():"<<m_username.Left(m_username.FindOneOf("@"))<<endl;	
// cout<<"sizeof(int):"<<sizeof(int)<<",m_formatsring:"<<m_formatsring<<endl<<"temp1:"<<temp1<<",m_md5:"<<m_md5<<endl<<"m_realusername:"<<m_realusername<<", m_md5use:"<< m_md5use<<endl;
// #endif
	return m_realusername;
}


bool  Rdial::CreateRASLink()
{
	LPRASENTRY lpRasEntry = NULL;
	DWORD cb = sizeof(RASENTRY);
	DWORD dwBufferSize = 0;
	DWORD dwRet = 0;

	//  ȡ��entry�Ĵ�С,���Ҳ��֪���ǲ��Ǳ����,��Ϊsizeof(RASENTRY)������ȡ����dwBufferSize��һ����,��������Getһ�°�ȫ�� 
	RasGetEntryProperties(NULL, "", NULL, &dwBufferSize, NULL, NULL); 
	if (dwBufferSize == 0)
		return false ;

	lpRasEntry = (LPRASENTRY)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBufferSize);
	if (lpRasEntry == NULL)
		return false ;

	ZeroMemory(lpRasEntry, sizeof(RASENTRY));
	lpRasEntry->dwSize		= dwBufferSize;
	lpRasEntry->dwfOptions  = RASEO_RemoteDefaultGateway|RASEO_PreviewPhoneNumber|RASEO_PreviewUserPw;  // ����ļ���ѡ��ͦ��Ҫ�ģ� RASEO_RemoteDefaultGateway���ѡ��Ѵ�������������ΪĬ�����ӣ� RASEO_PreviewPhoneNumber ��Ӧѡ���е���ʾ����绰���룬RASEO_PreviewUserPw��Ӧѡ���е���ʾ�û��������� 
	lpRasEntry->dwType		= RASET_Internet;

	lstrcpy(lpRasEntry->szDeviceType, RASDT_PPPoE);
	lstrcpy(lpRasEntry->szDeviceName, "connect");
	lpRasEntry->dwfNetProtocols   = RASNP_Ip;
	lpRasEntry->dwFramingProtocol = RASFP_Ppp;

	dwRet = RasSetEntryProperties(NULL, "connect", lpRasEntry, dwBufferSize, NULL, 0);  //  �������� 
	// The RasSetEntryProperties function changes the connection information for an entry in the phone book or creates a new phone-book entry.��reference MSDN��
	HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasEntry);

	if (dwRet != 0)
		return false;
	return true;
}


void banner()
{
	printf("\
 ____     _ _       _ \n\
|  _ \\ __| (_) __ _| |\n\
| |_) / _` | |/ _` | |\n\
|  _ < (_| | | (_| | |\n\
|_| \\_\\__,_|_|\\__,_|_|\t(alpha-1.1)\n\n\
    Author: Purpleroc@0xfa.club		\n\
    Url:    http://purpleroc.com    |\n\
    Email:  admin@0xfa.club         |\n\
    Update: 2016-01-08              |\n\
------------------------------------|\n");
}


int main (int argc,char **argv)
{
	banner();
	if (argc != 3)
	{
		printf("Parameter Error!\nUseage:rdial [username] [password]\n");
		system("pause");
		exit(0);
	}

	Rdial real(argv[1]);

	// ������Ϊconnect�Ĳ���ʵ��
	if (!real.CreateRASLink())
	{
		printf("Create Entry Failed!\n");
		exit(0);
	}

	// ͬ�����÷�ʽ
	RASDIALPARAMS RasDialParams;
	HRASCONN m_hRasconn;
	// ��������dwSize ΪRASDIALPARAMS�ṹ�Ĵ�С
	RasDialParams.dwSize = sizeof(RASDIALPARAMS);
	m_hRasconn = NULL; 

	// ����szEntryNameΪ���ַ���������RasDialʹ��ȱʡ��������
	_tcscpy(RasDialParams.szEntryName, _T("connect")); 
	RasDialParams.szPhoneNumber[0] =  _T('\0');
	RasDialParams.szCallbackNumber[0] = _T('\0');
	_tcscpy(RasDialParams.szUserName, _T(real.Realusername()));
	_tcscpy(RasDialParams.szPassword, _T(argv[2]));
	RasDialParams.szDomain[0] =  _T('\0');

	// ͬ����ʽ����RasDial(���������ΪNULL)
	DWORD Ret = RasDial(NULL, NULL, &RasDialParams, 0, NULL, &m_hRasconn);
	if (Ret != 0) 
	{ 
		TCHAR szBuff[MAX_PATH];
		_stprintf(szBuff,_T("RasDialʧ��: Error = %d\n"), Ret);
		OutputDebugString(szBuff);
		printf (szBuff);
		return 1;
	}
	else
	{
		printf("ʹ�� %s ���ӳɹ�\n",argv[1]);
		return 0;
	}
}
