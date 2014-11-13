///////////////////////////////////////////////////////////////////////////
//    HandleTCPClient.c
//  (For FakeCluster)
//  receives trigger signals from LOOPBACK UDP 
//  sends data through TCP
//
//
///////////////////////////////////////////////////////////////////////
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */

#define SNDBUFSIZE 976   /* Size of sending buffer */
#include <string.h> // for memset
#include <time.h>

//for UDP connection, probably neccessary
#include <sys/types.h>
#include <netdb.h>
#include "Practical.h"

#include <arpa/inet.h>//for inet_addr

void HandleTCPClient(int clntSocket, char *trigAcptPort, int PrintStatus)
{

  unsigned char sndBuffer[SNDBUFSIZE];/* Buffer for data string to be sent */
  memset(sndBuffer,73,sizeof(sndBuffer));
  /***************************************/
  //  Signal receiver port(SERVER) 
  /***************************************/
  // Definition
  if(PrintStatus==1)
    printf("FakeCluster1 : Trigger receive port is set to # %s\n",trigAcptPort);
  //tell the system what kinds of address we want
  //**************
  // ai_family  
  //  AF_UNSPEC :Any address family
  //  AF_INET   :IPv4 address family
  //  
  // ai_flags
  //  AI_PASSIVE    :Accept on any address/port */ 
  //  AI_NUMERICHOST:Accept only on numeric address/port
  //  
  //  
  //**************
  struct addrinfo addrCriteria;                     // Criteria for address
  memset(&addrCriteria, 0, sizeof(addrCriteria));   // Zero out structure
  addrCriteria.ai_family   = AF_INET;               // IPv4 address family
  addrCriteria.ai_flags    = AI_PASSIVE;            // Accept on any address/port */
  addrCriteria.ai_socktype = SOCK_DGRAM;            // Only datagram socket
  addrCriteria.ai_protocol = IPPROTO_UDP;           // Only UDP socket
  /***GETADDRINFO***/
  struct addrinfo *servAddr;                        // List of server addresses
  int rtnVal = getaddrinfo(NULL, trigAcptPort, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));
  /***SOCKET***/
  int trigSock = socket(servAddr->ai_family, servAddr->ai_socktype,
      servAddr->ai_protocol);
  if (trigSock < 0)
    DieWithSystemMessage("socket() failed");

  int yes =1;
  if(setsockopt(trigSock, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes,
		sizeof(yes))<0)
    DieWithSystemMessage("setsockopt() failed");
  /***BIND***/
  if (bind(trigSock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
    DieWithSystemMessage("bind() failed");
  
  // Free address list allocated by getaddrinfo()
  freeaddrinfo(servAddr);


  /***************************************/
  //  Handling process
  /***************************************/
  for(;;)
    {
      /***preparation for trigger acception***/
      // Client address
      struct sockaddr_storage clntAddr; 
      // Set Length of client address structure (in-out parameter)
      socklen_t clntAddrLen = sizeof(clntAddr);
      // Size of trigger signal message
      char buffer[MAXSTRINGLENGTH]; // I/O buffer

      /***trigger acception***/
      // (Block until receive message from a client)
      ssize_t numBytesRcvd = recvfrom(trigSock, buffer, MAXSTRINGLENGTH, 0,
				      (struct sockaddr *) &clntAddr, &clntAddrLen);
      if (numBytesRcvd < 0)
	DieWithSystemMessage("recvfrom() failed");
      
      /***trigger acception message***/
      if(PrintStatus==1)
	{
	  fputs("Trigger signal is accepted from ", stdout);
	  PrintSocketAddress((struct sockaddr *) &clntAddr, stdout);
	  fputc('\n', stdout);
	}
      /***sending data***/   
      if(send(clntSocket, sndBuffer, sizeof(sndBuffer), 0)<0)
	break;//DieWithError("Send error \n");
    }
  /* Close client socket */
  close(trigSock);      
  close(clntSocket);    
}
