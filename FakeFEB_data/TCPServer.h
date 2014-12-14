#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void DieWithError(char *errorMessage);
void HandleTCPClient(int clntSocket,char *trigAcptPort, int PrintStatus);
int CreateTCPServerSocket(unsigned short port);
int AcceptTCPConnection(int servSock);

