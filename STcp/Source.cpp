#include "CMyTcp.h"
int main()
{
    CMyTcp tcpServer;
    tcpServer.Accept("127.0.0.1", 50050);
    std::cout << "�������ֽ��������ӳɹ�!" << std::endl;
    return 0;
}