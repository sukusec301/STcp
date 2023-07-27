#pragma once
#include <vector>
class CBytesStreamBuffer
{
public:
    CBytesStreamBuffer();
    ~CBytesStreamBuffer();
    
    // 写入缓冲区，附加在缓冲区末尾
    void Write(const char* pBuffer, int nSize);
    // 从缓冲区读取指定字节数，读取的数据自动从缓冲区删除
    void Read(char* pBuffer, int nSize);
    // 从缓冲区读取指定字节数，不会删除
    void Peek(char* pBuffer, int nSize);
    // 获取缓冲区中数据的大小
    int GetSize() const;
    int GetSize();
private:
    std::vector<char> m_vectBuffer;
};

