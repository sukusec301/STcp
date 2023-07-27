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
    // 包信息
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
        WORD m_nPt;                   // 包类型
        DWORD m_nDataLen = 0;             // 数据长度
        DWORD m_nSeq = 0;                 // 包的序号
        DWORD m_nCheckSum = 0;            // 包校验值
        BYTE m_aryData[DATA_LEN];     // 包数据
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
    DWORD m_NextSendSeq;            // 下次拆包的序号
    DWORD m_NextRecvSeq;            // 下次存入缓冲区的包开始序号
private:

    std::map<DWORD, PackInfo> m_mapSend;            // 存储发送包的容器
    CLock m_lockSendMap;                            // 发包的容器被发包线程和收包线程共同操作了
    std::map<DWORD, PackInfo> m_mapRecv;            // 存储收包的容器
    CLock m_lockRecvMap;                            // 发包的容器被发包线程和收包线程共同操作了
    CBytesStreamBuffer m_bufRecv;                   // 接收数据的缓冲区
    CLock m_lockForBufRecv;                         // 发包的容器被发包线程和收包线程共同操作了
    // 工作线程

private:
    static DWORD CALLBACK SendThreadProc(LPVOID lpParam);       // 发包的线程
    static DWORD CALLBACK RecvThreadProc(LPVOID lpParam);       // 收包的线程
    static DWORD CALLBACK WorkThreadProc(LPVOID lpParam);       // 将包从收包容器丢入缓冲区的线程
};

