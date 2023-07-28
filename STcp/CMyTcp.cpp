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
    m_addrServer.sin_addr.S_un.S_addr = inet_addr(szIP);
    m_addrServer.sin_port = htons(dwPort);
    m_addrServer.sin_family = AF_INET;

    int nRet = bind(m_socketServerandClient, (sockaddr*)&m_addrServer, sizeof(m_addrServer));
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
        int nSizeofClient = sizeof(m_addrClient);
        int nRecvBytes = recvfrom(m_socketServerandClient, (char*)&recvPkg, sizeof(recvPkg), 0, (sockaddr*)&m_addrClient, &nSizeofClient);
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
        int nRet = sendto(m_socketServerandClient, (const char*)&ReplyACKPkg, sizeof(ReplyACKPkg), 0, (sockaddr*)&m_addrClient, sizeof(m_addrClient));
        if (nRet == SOCKET_ERROR)
        {
            continue;
        }

        // ���յڶ�����
        nRecvBytes = recvfrom(m_socketServerandClient, (char*)&recvPkg, sizeof(recvPkg), 0, (sockaddr*)&m_addrClient, &nSizeofClient);
        if (nRecvBytes == 0 || nRecvBytes == SOCKET_ERROR || recvPkg.m_nPt != PT_ACK || recvPkg.m_nSeq != m_NextRecvSeq)
        {
            continue;
        }
        // ��������
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
    m_addrServer.sin_addr.S_un.S_addr = inet_addr(szIP);
    m_addrServer.sin_port = htons(dwPort);
    m_addrServer.sin_family = AF_INET;
    // ģ����������: ����˽����������ݰ�, �ͻ��˷���һ�����ݰ�
    while (true)
    {
        // �ͻ��˷��͵�һ����: SYN
        Package SendPkg(PT_SYN, m_NextSendSeq);
        int nRet = sendto(m_socketServerandClient, (const char*)&SendPkg, sizeof(SendPkg), 0, (sockaddr*)&m_addrServer, sizeof(m_addrServer));
        if (nRet == SOCKET_ERROR)
        {
            ErrorProc("Client sendto the 1st package");
            return FALSE;
        }

        // �յ�һ����: SYN | ACK
        Package RecvPkg;
        int nSizeofaddrServer = sizeof(m_addrServer);
        int nRecvBytes = recvfrom(m_socketServerandClient, (char*)&RecvPkg, sizeof(RecvPkg), 0, (sockaddr*)&m_addrServer, &nSizeofaddrServer);
        // pkg.m_nSeq != m_NextRecvSeq ��ֹ���˲��
        if (nRecvBytes == 0 || nRecvBytes == SOCKET_ERROR || RecvPkg.m_nPt != (PT_SYN | PT_ACK) || RecvPkg.m_nSeq != m_NextRecvSeq)
        {
            ErrorProc("client recvfrom");
            closesocket(m_socketServerandClient);
            return FALSE;
        }

        // ���ڶ�����: ACK
        Package SendPkg2(PT_ACK, m_NextSendSeq);
        nRet = sendto(m_socketServerandClient, (const char*)&SendPkg2, sizeof(SendPkg2), 0, (sockaddr*)&m_addrServer, sizeof(m_addrServer));
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

    return TRUE;
}

DWORD CMyTcp::Send(LPBYTE pBuff, DWORD nBuffLen)
{
    // ������ݰ�����������
    DWORD dwCount = (nBuffLen % MSS == 0 ? nBuffLen / MSS : nBuffLen / MSS + 1);
    //DWORD dwCount = (nBuffLen + MSS) / MSS;
    m_lockSendMap.Lock();
    for (int i = 0; i < dwCount; ++i)
    {
        DWORD dwLen = MSS;
        // ���һ��������ʣ��ĳ���
        if (i == dwCount - 1)
        {
            dwLen = (nBuffLen - i * MSS);
        }
        Package package(PT_PSH, m_NextSendSeq, pBuff + i * MSS, dwLen);

        m_mapSend[m_NextSendSeq] = PackInfo(time(NULL), package);   // ��ʼ��ʱ��ȫ��Ϊ0��ȫ���㳬ʱ�İ�

        fprintf(stdout, "package->map seq: %d", m_NextSendSeq);
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
        for (auto& pi : pThis->m_mapSend)
        {
            // ������ط�



            // ��ʱ���ط�
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

    }
    return 0;
}

DWORD CMyTcp::WorkThreadProc(LPVOID lpParam)
{
    CMyTcp* pThis = (CMyTcp*)lpParam;
    while (true)
    {

    }
    return 0;
}
