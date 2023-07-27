#pragma once
#include <vector>
class CBytesStreamBuffer
{
public:
    CBytesStreamBuffer();
    ~CBytesStreamBuffer();
    
    // д�뻺�����������ڻ�����ĩβ
    void Write(const char* pBuffer, int nSize);
    // �ӻ�������ȡָ���ֽ�������ȡ�������Զ��ӻ�����ɾ��
    void Read(char* pBuffer, int nSize);
    // �ӻ�������ȡָ���ֽ���������ɾ��
    void Peek(char* pBuffer, int nSize);
    // ��ȡ�����������ݵĴ�С
    int GetSize() const;
    int GetSize();
private:
    std::vector<char> m_vectBuffer;
};

