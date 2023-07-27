#pragma once
#include "Common.h"
class CLock
{
public:
    CLock();
    ~CLock();
    void Lock();
    void Unlock();
private:
    CRITICAL_SECTION m_csLock;
};

