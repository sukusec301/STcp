#pragma once
#include <map>
#include "Common.h"
#include "CBytesStreamBuffer.h"
enum PackageType
{
    PT_PSH = 1,
    PT_ACK = 2,
    PT_FIN = 4,
    PT_SYN = 8,
};
class CMyTcp
{
public:
    
    BOOL Accept(LPCSTR szIP, USHORT dwPort);
    BOOL Connect(LPCSTR szIP, USHORT dwPort);

    DWORD Send(LPBYTE pBuff, DWORD nBuffLen);
    DWORD Recv(LPBYTE pBuff, DWORD nBuffLen);
    VOID Close();

    static VOID ErrorProc(const char* prev);
    // package info
private:
#pragma pack(push, 1)
    typedef struct tagPackage
    {
        tagPackage() = default;
        // to make ACK, give 3 default params
        tagPackage(WORD nPt,  DWORD nSeq = 0, BYTE* pData = nullptr, DWORD nDataLen = 0)
            : m_nPt{ nPt }
            , m_nSeq{ nSeq }
        {
            ZeroMemory(m_aryData, MSS);
            if (pData != NULL)
            {
                memcpy(m_aryData, pData, nDataLen);
                m_nDataLen = nDataLen;
                m_nCheckSum = CCrc32::crc32(m_aryData, m_nDataLen);
            }
        }
        WORD m_nPt;                         // package type
        DWORD m_nSeq = 0;                   // package sequence
        BYTE m_aryData[MSS];           // data
        DWORD m_nDataLen = 0;               // data len
        DWORD m_nCheckSum = 0;              // package sum
    }Package;
#pragma pack(pop)

    typedef struct tagPackInfo
    {
        tagPackInfo() = default;
        tagPackInfo(time_t LastTime, Package pkg) 
            : m_LastTime{ LastTime }, m_pkg{ pkg }{};
        time_t m_LastTime;
        Package m_pkg;
    }PackInfo;
private:
    SOCKET m_socketServerandClient;
    sockaddr_in m_addrServer;
    sockaddr_in m_addrClient;
private:
    DWORD m_NextSendSeq = 0;            // 下次拆包的序号
    DWORD m_NextRecvSeq = 0;            // 下次存入缓冲区的包开始序号
private:
    std::map<DWORD, PackInfo> m_mapSend;            // 存储发送包的容器
    CLock m_lockSendMap;                            // 发包的容器被发包线程和收包线程共同操作了
    std::map<DWORD, PackInfo> m_mapRecv;            // 存储收包的容器
    CLock m_lockRecvMap;                            // 发包的容器被发包线程和收包线程共同操作了
    CBytesStreamBuffer m_bufRecv;                   // 接收数据的缓冲区
    CLock m_lockForBufRecv;                         // 发包的容器被发包线程和收包线程共同操作了
    // 工作线程
    BOOL m_bWorking = FALSE;
    HANDLE m_hWorkThread = nullptr;
    HANDLE m_hRecvThread = nullptr;
    HANDLE m_hSendThread = nullptr;
private:
    static DWORD CALLBACK SendThreadProc(LPVOID lpParam);       // 发包的线程
    static DWORD CALLBACK RecvThreadProc(LPVOID lpParam);       // 收包的线程
    static DWORD CALLBACK WorkThreadProc(LPVOID lpParam);       // 将包从收包容器丢入缓冲区的线程
    BOOL AfterConnectInit();
};

