
#include "TCPClientSocket.hpp"

unsigned char __g_buff[65535];

using namespace LSTDAQ::LIB;

int main()
{
  char szAddr[16]="192.168.10.1";
  unsigned short shPort = 24;
  unsigned long lConnected=0;
  
  TCPClientSocket *tcps;
  tcps->connectTcp(szAddr,shPort,lConnected);
  tcps->readSock(__g_buff,976);
  tcps->closeSock();
  
  
}
