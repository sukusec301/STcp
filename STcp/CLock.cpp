#include "CLock.h"

CLock::CLock()
{
    InitializeCriticalSection(&m_csLock);
}

CLock::~CLock()
{
    DeleteCriticalSection(&m_csLock);
}

void CLock::Lock()
{
    EnterCriticalSection(&m_csLock);
}

void CLock::Unlock()
{
    LeaveCriticalSection(&m_csLock);
}
