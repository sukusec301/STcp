#include "CMyTcp.h"
int main()
{
    CMyTcp tcpServer;
    tcpServer.Accept("127.0.0.1", 50050);



    return 0;
}