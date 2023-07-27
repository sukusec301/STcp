#include "CInitSocketVersion.h"
CInitSocketVersion::CInitSocketVersion()
{
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    WSAStartup(wVersionRequested, &wsaData);
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        WSACleanup();
        return;
    }
}
CInitSocketVersion::~CInitSocketVersion()
{
    WSACleanup();
}
static CInitSocketVersion g_stVersion;