/******************************/
// To be checked:
// read() in the RB may not be implemented to read 1 event exactly.
//
//
/******************************/
#define MAX_RINGBUF 10
#define MAX_CONNECTION 48
#define DAQ_NEVENT  3

#include <iostream> //cout
#include <string.h> //strcpy
//#include <string>
#include <stdio.h>  //sprintf
#include <unistd.h> //for sleep()
#include <fstream>  //filestream(file I/O)
#include <sstream>  //stringstream
#include <iomanip>  //padding cout
#include <stdlib.h> //exit(1)

#include <pthread.h>
#include "RingBuffer.hpp"
#include "TCPClientSocket.hpp"
#include "DAQtimer.hpp"
using namespace std;

/****************************/
// struct definition
/****************************/
struct sRingBuffer{
  int sRBid;
  char szAddr[18];//="192.168.10.1"
  unsigned short shPort;// = 24;
  LSTDAQ::RingBuffer* rb ;
  sRingBuffer* next;
};
sRingBuffer sRB[MAX_RINGBUF];

//void sRBinit();
//void sRBcreate(int nServ);
//void sRBsetaddr(int sRBid, char *szAddr, unsigned short shPort);

void sRBinit()
{
  for(int i=0; i<MAX_RINGBUF; i++)
  {
    sRB[i].sRBid = -1;
    sRB[i].next = 0;
    cout <<sRB[i].next<<endl;
  }
}

void sRBcreate(int nServ)
{
  for(int i=0; i<nServ; i++)
  {
    sRB[i].sRBid = i;
    sRB[i].rb = new LSTDAQ::RingBuffer();
    sRB[i].next=&sRB[i+1];
    cout <<sRB[i].next<<endl;
  }
}

void sRBsetaddr(int sRBid, char *szAddr, unsigned short shPort)
{
  if(sRB[sRBid].sRBid==sRBid)
  {
    strcpy(sRB[sRBid].szAddr, szAddr);
    sRB[sRBid].shPort=shPort;
  }
}


/****************************/
// thread definition
/****************************/
void *Collector_thread(void *arg)
{
  //receive buffer(From socket to ringbuffer)
  char tempbuf[976];
  //ring buffer(From receive buffer to Builder)
  sRingBuffer *srb = (sRingBuffer*)arg;
  
  //connection
  unsigned long lConnected = 0;
  LSTDAQ::LIB::TCPClientSocket *tcps;
  tcps = new LSTDAQ::LIB::TCPClientSocket();
  tcps->connectTcp(srb->szAddr,srb->shPort,lConnected);
  
  //sleep(1);
  for(int i = 0; i< DAQ_NEVENT;i++)
//  while(1)
  {
    tcps->readSock(tempbuf,976);
    srb->rb->write((void*)tempbuf);
    cout << "Coll"<<srb->sRBid <<" wrote :"<< tempbuf <<endl;
  }

}


void *Builder_thread(void *arg)
{
  sRingBuffer *srb[MAX_RINGBUF];
  srb[0]= (sRingBuffer*)arg;
  char tempbuf[976];
  unsigned long long ReadBytes = DAQ_NEVENT * 976;
  
  //variables for measurement
  unsigned long long llMesReadBytes[MAX_RINGBUF] = {0};
  
  cout<<"*** Builder_thread initialization ***"<<endl;
  int nColl =0;
  while(1)
  {
    cout<<"srb["<<nColl<<"] :"<<srb[nColl]->next<<endl;
    if(srb[nColl]->next==0||nColl==MAX_RINGBUF)break;
    srb[nColl+1]=srb[nColl]->next;
    nColl++;
  }
  
  LSTDAQ::DAQtimer *dt=new LSTDAQ::DAQtimer(nColl);
  dt->DAQstart();
  while(1)
  {
    for(int i =0;i<nColl;i++)
    {
      llMesReadBytes[i] +=srb[i]->rb->read((void*)tempbuf);
      dt->readend();
      //cout << "Bld read from Coll"<<srb[i]->sRBid <<" :"<< tempbuf <<endl;
      
    }
    if(llMesReadBytes[nColl-1]>=ReadBytes)break;
  }
  dt->DAQend();
  dt->DAQsummary();
}
/****************************/
// main definition
/****************************/
int main()
{
  cout << "Hello world!!init" <<endl;
  /******************************************/
  //  Reading Connection Configuration
  /******************************************/
  char szAddr[MAX_CONNECTION][16];
  unsigned short shPort[MAX_CONNECTION]={0};
  unsigned long lConnected=0;
  const char *ConfFile = "Connection.conf";
  std::ifstream ifs(ConfFile);
  std::string str;
  int nServ=0;
  while (std::getline(ifs,str)){
    if(str[0]== '#' || str.length()==0)continue;
    std::istringstream iss(str);
    iss >> szAddr[nServ] >> shPort[nServ];
    nServ++;
  }
  if(nServ>MAX_CONNECTION){
    printf("The number of connections excessed limit.");
    exit(1);
  }
  
  sRBinit();
  sRBcreate(nServ);
  for(int i=0;i<nServ;i++)
    sRBsetaddr(i,szAddr[i],shPort[i]);
  
  cout<<"****** Configuration of RinbBuffers are set ******"<<endl;
  cout<<"RBid "<<"    IP address     "<<" port "<<endl;
  for(int i=0;i<nServ;i++)
    cout<<"RB"<< setw(2) << setfill(' ')<<i<<"|"<<
    setw(18) << setfill(' ')<<sRB[i].szAddr<<"|"<<
    setw(5) << setfill(' ')<<sRB[i].shPort<<endl;
  
  /******************************************/
  //  Multi-threaded process creation
  /******************************************/
  cout << "***  Thread create  ***"<<endl;
  pthread_t handle[nServ+1];
  for(int i=0;i<nServ;i++)
  {
    pthread_create(&handle[i],
                    NULL,
                    &Collector_thread,
                    &sRB[i]);
  }
  //sleep(1);
  pthread_create(&handle[nServ],
                  NULL,
                  &Builder_thread,
                  &sRB[0]);
  cout << DAQ_NEVENT << "events will be transferred each"<<endl;
  for(int i=0;i<nServ+1;i++)
    pthread_join(handle[i],NULL);
  
  cout << "Hello world!!end" <<endl;

  return 0;
}

