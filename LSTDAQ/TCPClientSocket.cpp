//#include <errno.h>//for ???
#include  <unistd.h>//for read()
#include  <stdio.h>//for perror, printf
//#include  <string.h>
//#include  <time.h>
//#include  <iostream>
//#include  <stdlib.h>
#include  <string.h>//for memset
#include "RingBuffer.hpp"
#include "TCPClientSocket.hpp"

namespace LSTDAQ{
  namespace LIB{
    
    //constructor
    TCPClientSocket::TCPClientSocket() throw():m_sockTcp(-1)
    {
      memset( &m_addrTcp, 0, sizeof(sockaddr_in));
    }

    //copy constructor
    TCPClientSocket::TCPClientSocket
      (const TCPClientSocket &tcps) throw()
    {
      m_sockTcp                 = tcps.m_sockTcp;
      m_addrTcp.sin_family      = tcps.m_addrTcp.sin_family;
      m_addrTcp.sin_addr.s_addr = tcps.m_addrTcp.sin_addr.s_addr;
      m_addrTcp.sin_port        = tcps.m_addrTcp.sin_port;
      for( int i = 0;i<8;i++)
        m_addrTcp.sin_zero[i] = tcps.m_addrTcp.sin_zero[i];
      *this = tcps;
    }
    TCPClientSocket::~TCPClientSocket() throw()
    {
    }

    //=operator
    TCPClientSocket &TCPClientSocket::operator =
      (const TCPClientSocket &tcps) throw()
    {
      m_sockTcp                 = tcps.m_sockTcp;
      m_addrTcp.sin_family      = tcps.m_addrTcp.sin_family;
      m_addrTcp.sin_addr.s_addr = tcps.m_addrTcp.sin_addr.s_addr;
      m_addrTcp.sin_port        = tcps.m_addrTcp.sin_port;
      for( int i = 0;i<8;i++)
        m_addrTcp.sin_zero[i] = tcps.m_addrTcp.sin_zero[i];
      return *this;
    }
    //setter & getter
    int TCPClientSocket::getSock() throw()
    {
      return m_sockTcp;
    }
    //connect method
    bool TCPClientSocket::connectTcp(const char *pszHost,
                    unsigned short shPort,
                    unsigned long &lConnectedIP )
    {
      //int sockTcp = -1;
      //struct sockaddr_in addrTcp;
      m_sockTcp = socket(AF_INET, SOCK_STREAM, 0);
      if( m_sockTcp < 0 )
      {
        perror("socket() error");
        return -1;
      }
      m_addrTcp.sin_family = AF_INET;
      m_addrTcp.sin_port = htons(shPort);
      m_addrTcp.sin_addr.s_addr = inet_addr(pszHost);
      if( m_addrTcp.sin_addr.s_addr == 0xffffffff )
      {
        struct hostent *host = gethostbyname(pszHost);
        if( host == NULL )
        {
          if( h_errno == HOST_NOT_FOUND )
          {
            printf("ConnectTcp() host not found : %s\n",pszHost);
          }
          else
          {
            printf("ConnectTcp() %s : %s\n",hstrerror(h_errno),pszHost);
            fprintf(stderr,"%s : %s\n",hstrerror(h_errno),pszHost);
          }
          return -2;
        }
        unsigned int **addrptr = (unsigned int **)host->h_addr_list;
        while( *addrptr != NULL)
        {
          m_addrTcp.sin_addr.s_addr = *(*addrptr);
          if(connect(m_sockTcp,
                    (struct sockaddr *)&m_addrTcp,
                    sizeof(m_addrTcp))
             ==0 )
          {
              lConnectedIP = (unsigned long )m_addrTcp.sin_addr.s_addr;
              break;
          }
          addrptr++;
        }
        if( addrptr == NULL)
        {
          perror("ConnectTCP()::connect(1)");
          printf("ERROR:ConnectTCP:: host not found (%d) to %s:%u\n",
                 m_sockTcp,pszHost,shPort);
          return -3;
        }
        else
        {
          printf("ConnectTCP::Connected0(%d) to %s:%u\n",
                 m_sockTcp,pszHost,shPort);
        }
      }
      else
      {
        if(connect(m_sockTcp,(struct sockaddr *)&m_addrTcp,sizeof(m_addrTcp)) !=0 )
        {
            perror("ConnectTCP()::connect(2)");
            printf("ERROR:ConnectTCP:: can't connect not found (%d) to %08X %s:%u\n",
                   m_addrTcp.sin_addr.s_addr,m_sockTcp,pszHost,shPort);
            m_sockTcp = -1;
            close(m_sockTcp);
            return -4;
        }
        else
        {
          lConnectedIP = (unsigned long )m_addrTcp.sin_addr.s_addr;
          printf("ConnectTCP::Connected1(%d) %s:%d\n",m_sockTcp,pszHost,shPort);
        }
      }
          return 0;      //return( sockTcp );
    }
    ssize_t TCPClientSocket::readSock(void *buffer, size_t nbytes) throw()
    {
        ssize_t n = read( m_sockTcp,buffer,nbytes);
        return n;
    }
    bool TCPClientSocket::closeSock()
    {
        return close(m_sockTcp);
    }

  }//namespace LIB
}//namespace LSTDAQ
