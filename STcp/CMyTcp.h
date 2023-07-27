#pragma once
#include "Common.h"
#include <map>
#include "CBytesStreamBuffer.h"

class CMyTcp
{
public:
    BOOL Accept(LPCSTR szIP, USHORT dwPort);
    BOOL Connect(LPCSTR szIP, USHORT dwPort);
    DWORD Send(LPBYTE pBuff, DWORD nBuffLen);
    DWORD Recv(LPBYTE pBuff, DWORD nBuffLen);
    VOID Close();
    // ����Ϣ
private:
#pragma pack(push, 1)
    typedef struct tagPackage
    {
        tagPackage(WORD nPt, DWORD nDataLen, DWORD nSeq, BYTE* pData)
            : m_nPt{ nPt }
            , m_nSeq{ nSeq }
        {
            ZeroMemory(m_aryData, DATA_LEN);
            if (m_aryData != NULL)
            {
                memcpy(m_aryData, pData, nDataLen);
                m_nDataLen = nDataLen;
                m_nCheckSum = CCrc32::crc32(m_aryData, m_nDataLen);
            }
        }
        WORD m_nPt;                   // ������
        DWORD m_nDataLen = 0;             // ���ݳ���
        DWORD m_nSeq = 0;                 // �������
        DWORD m_nCheckSum = 0;            // ��У��ֵ
        BYTE m_aryData[DATA_LEN];     // ������
    }Package;
#pragma pack(pop)

    typedef struct tagPackInfo
    {
        tagPackInfo(time_t LastTime, Package pkg) 
            : m_LastTime{ LastTime }, m_pkg{ pkg }{};
        time_t m_LastTime;
        Package m_pkg;
    }PackInfo;

private:
    DWORD m_NextSendSeq;            // �´β�������
    DWORD m_NextRecvSeq;            // �´δ��뻺�����İ���ʼ���
private:

    std::map<DWORD, PackInfo> m_mapSend;            // �洢���Ͱ�������
    CLock m_lockSendMap;                            // �����������������̺߳��հ��̹߳�ͬ������
    std::map<DWORD, PackInfo> m_mapRecv;            // �洢�հ�������
    CLock m_lockRecvMap;                            // �����������������̺߳��հ��̹߳�ͬ������
    CBytesStreamBuffer m_bufRecv;                   // �������ݵĻ�����
    CLock m_lockForBufRecv;                         // �����������������̺߳��հ��̹߳�ͬ������
    // �����߳�

private:
    static DWORD CALLBACK SendThreadProc(LPVOID lpParam);       // �������߳�
    static DWORD CALLBACK RecvThreadProc(LPVOID lpParam);       // �հ����߳�
    static DWORD CALLBACK WorkThreadProc(LPVOID lpParam);       // �������հ��������뻺�������߳�
};

