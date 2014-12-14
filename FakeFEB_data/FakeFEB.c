////////////////////////////////////////////////////////////////
// FakeCluster.c
//
//  DataSendPort      : default 24
//  TriggerAcceptPort : default 12345
//  
//  servSock(TCP)  :the socket to wait client with binded port
//  clntSock(TCP)  :the socket of port through which client requests to send data
//  TrigSock(UDP)  :
//
//
//
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
  unsigned short dataServPort;
  //  unsigned short trigAcptPort;
  char * trigAcptPort;
  pid_t processID;
  unsigned int childProcCount = 0;
  int PrintStatus = 1;
  /**********************************************/
  //  Handling input arguments
  /**********************************************/
  if (argc != 1)
    {
      if (argc !=3 && argc !=4)//if argv is invalid
	{
	  fprintf(stderr, "Usage: %s <DataServerPort> <TriggerAcceptPort> [1|0]\n", argv[0]);
	  exit(1);
	}
      // if argv is 2 or 3  
      dataServPort = atoi(argv[1]);
      trigAcptPort = argv[2];
      if(argc == 4) PrintStatus = atoi(argv[3]);
      printf("argument is used for connection\n");
    }
  else//if no argv
    {
      dataServPort=24;
      trigAcptPort="12345";/**Trigger accept port 12345***/
      PrintStatus=1;
      printf("connection will be accepted with default");
    }
  /**********************************************/
  //  Creating the socket to accept
  //                   a connection requiremet
  /**********************************************/
  servSock = CreateTCPServerSocket(dataServPort);
  
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
	  HandleTCPClient(clntSock,trigAcptPort,PrintStatus);
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
