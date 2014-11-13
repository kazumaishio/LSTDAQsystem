////////////////////////////////////////////////////////////////
// FakeCluster.c
//   By Kazuma Ishio.
//   Last update : 2014/08/16
//  ****Outline****
//  This program will send event data
//  [From]
//  DataServeAddress   : default 192.168.10.9
//  DataServePort      : default 24
//  [To] the address which requested.
//  
//  The trigger signal will be accepted  with
//  TriggerReceivePort : default 30001
//  
//  ****Usage****
//  compile:
//    gcc -o FakeCluster -std=gnu99 -lrt 
//          FakeCluster.c     AcceptTCPConnection.c  
//          AddressUtility.c  DieWithMessage.c 
//          DieWithError.c    CreateTCPServerSocket.c 
//          HandleTCPClient.c
//  submit:
//    sudo ./FakeCluster <DataDepartureIP> <DataDeparturePort> <TriggerReceivePort> [1|0]
//    sudo ./FakeCluster 192.168.10.9      24                  30001                 
//   or just run with
//    sudo ./FakeCluster
//  The last argurement is whether you want full output for running status.
//  ('1' is "yes")
//  
//  ****Flow of the program****
//  (1) main calls CreateTCPServerSocket
//     to make a socket as a DataServer.
//  (2) When a request comes,
//  (3) main calls AcceptTCPConnection and HandleTCPClient.
//  (4) A child process of HandleTCPClient is made by fork.
//  (5) HandleTCPClient makes TrigSock
//       TrigSock receive trigger as LOOPBACK packets.
//       clntSock throws data.
//    
//  ****socket descriptors in this program****
//  servSock(TCP)  :the socket to wait client with binded port
//  clntSock(TCP)  :the socket of port through which client requests to send data
//  TrigSock(UDP)  :the socket to accept trigger signal packets through UDP.
//  
//  
//  
////////////////////////////////////////////////////////////////

#include "TCPServer.h"
#include <sys/wait.h>

int main(int argc, char *argv[])
{
  int servSock;
  int clntSock;
  char *         dataServAddr;
  unsigned short dataServPort;
  char *         trigRecvPort;
  pid_t processID;
  unsigned int childProcCount = 0;
  int PrintStatus = 0;
  /**********************************************/
  //  Handling input arguments
  /**********************************************/
  if (argc != 1)
    {
      if (argc !=4 && argc !=5)//if argv is invalid
	{
	  fprintf(stderr, 
		  "Usage: %s <DataDepartureIP> <DataDeparturePort> <TriggerReceivePort> [1|0]\n", argv[0]);
	  exit(1);
	}
      // if argc is 4 or 5 
      dataServAddr = argv[1];
      dataServPort = atoi(argv[2]);
      trigRecvPort = argv[3];
      if(argc == 5) PrintStatus = atoi(argv[4]);
      printf("argument is used for connection\n");
    }
  else//if no argv
    {
      dataServAddr="192.168.10.9";
      dataServPort=24;
      trigRecvPort="30001";/**Trigger receive port 12345***/
      PrintStatus=1;
      printf("Connection will be accepted with default\n");
    }
  printf("Data will be served from %s:%u\n",dataServAddr,dataServPort);
  printf("Trigger will be accepted thorough port %s\n",trigRecvPort);

  /**********************************************/
  //  Creating the socket to accept
  //                   a connection requiremet
  /**********************************************/
  servSock = CreateTCPServerSocket(dataServAddr, dataServPort);
  
  /**********************************************/
  //  Waiting, Accepting , and fork process
  /**********************************************/
  for(;;)
    {
      clntSock = AcceptTCPConnection(servSock);
      if ((processID = fork()) < 0)
	{
	  DieWithError("fork() failed");
	}
      else if (processID == 0 )
	{
	  close(servSock);
	  HandleTCPClient(clntSock,trigRecvPort,PrintStatus);
	  exit(0);
	}
      /***When child process is successfully generated***/
      printf("with child process: %d\n",(int)processID);
      close(clntSock);
      childProcCount++;

      while (childProcCount)
	{
	  processID = waitpid((pid_t) - 1, NULL, WNOHANG);
	  if(processID < 0)
	    DieWithError("waitpid() failed");
	  else if (processID ==0)
	    break;
	  else
	    childProcCount--;
	}
    }
}
