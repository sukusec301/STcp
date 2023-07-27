#include "CBytesStreamBuffer.h"

CBytesStreamBuffer::CBytesStreamBuffer()
{
}

CBytesStreamBuffer::~CBytesStreamBuffer()
{
}

void CBytesStreamBuffer::Write(const char* pBuffer, int nSize)
{
    for (int i = 0; i < nSize; ++i)
        m_vectBuffer.push_back(pBuffer[i]);
}

void CBytesStreamBuffer::Read(char* pBuffer, int nSize)
{
    memcpy(pBuffer, m_vectBuffer.data(), nSize);
    auto itBegin = m_vectBuffer.begin();
    auto itEnd = m_vectBuffer.end();
    m_vectBuffer.erase(itBegin, itEnd);
}

void CBytesStreamBuffer::Peek(char* pBuffer, int nSize)
{
    memcpy(pBuffer, m_vectBuffer.data(), nSize); // 取数据不删除
}

int CBytesStreamBuffer::GetSize() const
{
    return m_vectBuffer.size();
}

int CBytesStreamBuffer::GetSize()
{
    return m_vectBuffer.size();
}
