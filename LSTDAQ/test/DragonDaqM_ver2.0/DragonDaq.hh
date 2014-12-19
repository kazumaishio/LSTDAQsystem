#ifndef DRAGON_H
#define DRAGON_H

typedef struct{
  char *ipaddress;
  unsigned short portnum;
  unsigned int udp_sock;
  unsigned int tcp_sock;
  char *tcpAddr;
  //  struct sockaddr_in udpAddr;
  int is_open;
} eth_dragon;

int ConnectTcp(const char *pszHost, unsigned short shPort, unsigned long &lConnectedIP );
unsigned long long GetRealTimeInterval(const  struct timespec *pFrom, const struct timespec *pTo);
#endif
