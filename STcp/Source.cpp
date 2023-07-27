#include "CMyTcp.h"
int main()
{
    CMyTcp tcpServer;
    tcpServer.Accept("127.0.0.1", 50050);
    std::cout << "三次握手结束，连接成功!" << std::endl;
    return 0;
}