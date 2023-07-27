#pragma once
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <windows.h>
#include <WinSock2.h>
#define DATA_LEN 1460
#pragma comment(lib, "Ws2_32.lib")
#include "CLock.h"
#include "CCrc32.h"
enum PackageType
{
    PT_DATA = 1,
    PT_ACK = 2,
    PT_FIN = 4,
    PT_SYN = 8,
};