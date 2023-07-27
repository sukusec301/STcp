#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma pack(push,1)
class CPackage
{
public:
    CPackage() = default;
    CPackage(const CPackage& other);
    CPackage& operator=(const CPackage& other);
    ~CPackage() = default;
public:
    WORD nLen;
    DWORD checkSum;
    char szData[1];
};
#pragma pack(pop)
