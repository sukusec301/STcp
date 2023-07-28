#include "CMyTcp.h"

BOOL CMyTcp::Accept(LPCSTR szIP, USHORT dwPort)
{
    // Server init socket
    m_socketServerandClient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socketServerandClient == INVALID_SOCKET)
    {
        ErrorProc("socket");
        return FALSE;
    }

    // bind
    m_siSrc.sin_addr.S_un.S_addr = inet_addr(szIP);
    m_siSrc.sin_port = htons(dwPort);
    m_siSrc.sin_family = AF_INET;

    int nRet = bind(m_socketServerandClient, (sockaddr*)&m_siSrc, sizeof(m_siSrc));
    if (nRet == SOCKET_ERROR)
    {
        ErrorProc("bind");
        closesocket(m_socketServerandClient);
        return FALSE;
    }

    // ģ����������: ����˽����������ݰ�, �ͻ��˷���һ�����ݰ�
    while (true)
    {
        // ������յ�һ����: SYN
        Package recvPkg{};
        int nSizeofClient = sizeof(m_siDst);
        int nRecvBytes = recvfrom(m_socketServerandClient, (char*)&recvPkg, sizeof(recvPkg), 0, (sockaddr*)&m_siDst, &nSizeofClient);
        // pkg.m_nSeq != m_NextRecvSeq ��ֹ���˲��
        if (nRecvBytes == 0 || nRecvBytes == SOCKET_ERROR || recvPkg.m_nPt != PT_SYN || recvPkg.m_nSeq != m_NextRecvSeq)
        {
            continue;
            //ErrorProc("recvfrom");
            //closesocket(m_socketServer);
            //return FALSE;
        }

        // �ص�һ����: SYN | ACK
        Package ReplyACKPkg(PT_ACK | PT_SYN, m_NextSendSeq);
        int nRet = sendto(m_socketServerandClient, (const char*)&ReplyACKPkg, sizeof(ReplyACKPkg), 0, (sockaddr*)&m_siDst, sizeof(m_siDst));
        if (nRet == SOCKET_ERROR)
        {
            continue;
        }

        // ���յڶ�����
        nRecvBytes = recvfrom(m_socketServerandClient, (char*)&recvPkg, sizeof(recvPkg), 0, (sockaddr*)&m_siDst, &nSizeofClient);
        if (nRecvBytes == 0 || nRecvBytes == SOCKET_ERROR || recvPkg.m_nPt != PT_ACK || recvPkg.m_nSeq != m_NextRecvSeq)
        {
            continue;
        }
        // ��������
        Log("�������ֳɹ�����ʼ��ʼ��");
        break;
    }
    // �������Ӻ�ĳ�ʼ��: �������кš���ʼ׼�����գ����������߳�
    return AfterConnectInit();
}

// client!!
BOOL CMyTcp::Connect(LPCSTR szIP, USHORT dwPort)
{
    // Client init socket
    m_socketServerandClient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socketServerandClient == INVALID_SOCKET)
    {
        ErrorProc("socket");
        return FALSE;
    }
    m_siDst.sin_addr.S_un.S_addr = inet_addr(szIP);
    m_siDst.sin_port = htons(dwPort);
    m_siDst.sin_family = AF_INET;
    // ģ����������: ����˽����������ݰ�, �ͻ��˷���һ�����ݰ�
    while (true)
    {
        // �ͻ��˷��͵�һ����: SYN
        Package SendPkg(PT_SYN, m_NextSendSeq);
        int nRet = sendto(m_socketServerandClient, (const char*)&SendPkg, sizeof(SendPkg), 0, (sockaddr*)&m_siDst, sizeof(m_siDst));
        if (nRet == SOCKET_ERROR)
        {
            ErrorProc("Client sendto the 1st package");
            return FALSE;
        }

        // �յ�һ����: SYN | ACK
        Package RecvPkg;
        int nSizeofaddrServer = sizeof(m_siDst);
        int nRecvBytes = recvfrom(m_socketServerandClient, (char*)&RecvPkg, sizeof(RecvPkg), 0, (sockaddr*)&m_siDst, &nSizeofaddrServer);
        // pkg.m_nSeq != m_NextRecvSeq ��ֹ���˲��
        if (nRecvBytes == 0 || nRecvBytes == SOCKET_ERROR || RecvPkg.m_nPt != (PT_SYN | PT_ACK) || RecvPkg.m_nSeq != m_NextRecvSeq)
        {
            ErrorProc("client recvfrom");
            closesocket(m_socketServerandClient);
            return FALSE;
        }

        // ���ڶ�����: ACK
        Package SendPkg2(PT_ACK, m_NextSendSeq);
        nRet = sendto(m_socketServerandClient, (const char*)&SendPkg2, sizeof(SendPkg2), 0, (sockaddr*)&m_siDst, sizeof(m_siDst));
        if (nRet == SOCKET_ERROR)
        {
            ErrorProc("Client sendto the 2nd package");
            closesocket(m_socketServerandClient);
            return FALSE;
        }

        // ��������
        break;
    }

    // �������Ӻ�ĳ�ʼ��: �������кš���ʼ׼�����գ����������߳�
    return AfterConnectInit();
}

BOOL CMyTcp::AfterConnectInit()
{
    // �������к�
    m_NextRecvSeq++;
    m_NextSendSeq++;

    // ��ʼ׼�����գ����������߳�
    m_bWorking = TRUE;
    m_hSendThread = CreateThread(NULL, 0, SendThreadProc, this, 0, NULL);
    if (m_hSendThread == nullptr)
    {
        ErrorProc("SendThreadProc");
        closesocket(m_socketServerandClient);
        return FALSE;
    }
    m_hRecvThread = CreateThread(NULL, 0, RecvThreadProc, this, 0, NULL);
    if (m_hRecvThread == nullptr)
    {
        ErrorProc("SendThreadProc");
        closesocket(m_socketServerandClient);
        return FALSE;
    }
    m_hWorkThread = CreateThread(NULL, 0, WorkThreadProc, this, 0, NULL);
    if (m_hWorkThread == nullptr)
    {
        ErrorProc("SendThreadProc");
        closesocket(m_socketServerandClient);
        return FALSE;
    }

    CloseHandle(m_hWorkThread);
    CloseHandle(m_hRecvThread);
    CloseHandle(m_hSendThread);

    Log("�����̳߳ɹ�!");
    return TRUE;
}

DWORD CMyTcp::Send(LPBYTE pBuff, DWORD nBuffLen)
{
    if (nBuffLen <= 0)
    {
        return FALSE;
    }
    // ������ݰ�����������
    DWORD dwCount = (nBuffLen + MSS - 1) / MSS;
    // �Ƚ�low��д��: DWORD dwCount = (nBuffLen % MSS == 0 ? nBuffLen / MSS : nBuffLen / MSS + 1);

    m_lockSendMap.Lock();
    for (DWORD i = 0; i < dwCount; ++i)
    {
        DWORD dwLen = MSS;
        // ���һ��������ʣ��ĳ���
        if (i == dwCount - 1)
        {
            dwLen = (nBuffLen - i * MSS);
        }
        Package package(PT_PSH, m_NextSendSeq, pBuff + i * MSS, dwLen);

        m_mapSend[m_NextSendSeq] = PackInfo(0, package);   // ��ʼ��ʱ��ȫ��Ϊ0��ȫ���㳬ʱ�İ�
        //m_mapSend[m_NextSendSeq] = PackInfo(time(NULL), package);   

        Log("package----->map seq: %d", m_NextSendSeq);

        ++m_NextSendSeq;
    }
    m_lockSendMap.UnLock();
    return nBuffLen;
}

DWORD CMyTcp::Recv(LPBYTE pBuff, DWORD nBuffLen)
{
    return 0;
}

VOID CMyTcp::Close()
{
    return VOID();
}

VOID CMyTcp::ErrorProc(const char* prev)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR)&lpMsgBuf,
        0,
        NULL
    );
    // Process any inserts in lpMsgBuf.
    // ...
    // Display the string.
    fprintf(stdout, "[%s]: %s [ErrCode]%d", prev, (LPTSTR)lpMsgBuf, WSAGetLastError());
    // Free the buffer.
    LocalFree(lpMsgBuf);
    return VOID();
}

DWORD CMyTcp::SendThreadProc(LPVOID lpParam)
{
    CMyTcp* pThis = (CMyTcp*)lpParam;
    while (true)
    {
        pThis->m_lockSendMap.Lock();
        ULONGLONG nCurrentTime = GetTickCount64();    // ��64��û��������ޣ����������� ��64�����49.7��
        for (auto& pt : pThis->m_mapSend)
        {
            // ��ʱ���ط�
            if (( nCurrentTime - pt.second.m_LastTime) > RTO)
            {
                pThis->Log("[package]:%d retransmissions", pt.second.m_pkg.m_nSeq);
                sendto(pThis->m_socketServerandClient
                    , (char*)&pt.second.m_pkg, sizeof(pt.second.m_pkg)
                    , 0 , (sockaddr*)&pThis->m_siDst, sizeof(pThis->m_siDst));
                pt.second.m_LastTime = nCurrentTime;
                pt.second.m_pkg.m_nCount++;
            }

        }
        pThis->m_lockSendMap.UnLock();
    }
    return 0;
}

DWORD CMyTcp::RecvThreadProc(LPVOID lpParam)
{
    CMyTcp* pThis = (CMyTcp*)lpParam;
    while (true)
    {
        sockaddr_in si{};
        int nLen = sizeof(si);
        Package pkg{};
        int nRet = recvfrom(pThis->m_socketServerandClient, (char*)&pkg, sizeof(pkg),0
        ,(sockaddr*)&si, &nLen);
        if (nRet == 0 || nRet == SOCKET_ERROR)
        {
            continue;
        }
        switch (pkg.m_nPt)
        {
        case PT_ACK:    // ����Ӧ���ݰ��ӷ��Ͷ������Ƴ�
        {
            pThis->m_lockSendMap.Lock();
            pThis->m_mapSend.erase(pkg.m_nSeq);
            pThis->m_lockSendMap.UnLock();
            break;
        }
            
        case PT_PSH:    // У��ɹ�����ACK��У��ʧ�ܣ������հ�����
        {
            DWORD dwCheck = CCrc32::crc32(pkg.m_aryData, pkg.m_nDataLen);
            if (dwCheck != pkg.m_nCheckSum)
            {
                break;
            }
            // У��ɹ��ظ�ACK
            Package pkgAck(PT_ACK, pkg.m_nSeq);
            int nRet = sendto(pThis->m_socketServerandClient, (char*)&pkgAck, sizeof(pkgAck)
            , 0, (sockaddr*)&pThis->m_siDst, sizeof(pThis->m_siDst));
            //if (nRet == SOCKET_ERROR || nRet == 0) return FALSE;

            // ��������
            pThis->m_lockRecvMap.Lock();
            if (pThis->m_mapRecv.find(pkg.m_nSeq) != pThis->m_mapRecv.end() || // �����д���ŵİ��Ѿ�����
                pkg.m_nSeq < pThis->m_NextRecvSeq) // ����ŵİ��е������Ѿ����뻺����
            {
                pThis->m_lockRecvMap.UnLock();
                break;
            }
            pThis->m_mapRecv[pkg.m_nSeq] = pkg;
            pThis->m_lockRecvMap.UnLock();

            pThis->Log("package --- > m_mapRecv ok.");
            break;
        }

        default:
            break;
        }
    }
    return 0;
}

DWORD CMyTcp::WorkThreadProc(LPVOID lpParam)
{
    CMyTcp* pThis = (CMyTcp*)lpParam;
    while (pThis->m_bWorking)
    {
        pThis->m_lockForBufRecv.Lock();
        while (true)
        {
            pThis->m_lockRecvMap.Lock();

            // TODO: �ӽ��ն������ó����ݰ���������

            pThis->m_lockRecvMap.UnLock();
            
        }
        pThis->m_lockForBufRecv.UnLock();
    }
    return 0;
}


VOID CMyTcp::Log(const char* szFmt...)
{
    // TODO: Add your implementation code here.

    char szBuf[MAXBYTE]{};

    va_list vl;
    va_start(vl, szBuf);
    vsprintf(szBuf, szFmt, vl);
    va_end(vl);

    OutputDebugString(szBuf);


    return VOID();
}
