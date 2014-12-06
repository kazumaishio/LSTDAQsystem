#ifndef __TCP_CLIENT_SOCKET_H
#define __TCP_CLIENT_SOCKET_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


namespace LSTDAQ{
  namespace LIB{
class TCPClientSocket
{
public:
  //constructor & destructor
  TCPClientSocket         (                        ) throw();
  TCPClientSocket         ( const TCPClientSocket& ) throw();
  virtual ~TCPClientSocket(                        ) throw();
  //operator=
  TCPClientSocket &operator = (const TCPClientSocket&) throw();
  
  //setter & getter
  int getSock() throw();

  //method
  bool connectTcp(const char *pszHost,
                    unsigned short shPort,
                    unsigned long &lConnectedIP);
  ssize_t readSock(void *buffer, size_t nbytes) throw();

  bool closeSock();
  
private:
  int m_sockTcp;
  sockaddr_in m_addrTcp;
  int m_sockNo;
};
    
  }
}

#endif
