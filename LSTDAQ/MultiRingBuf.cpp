#define MAX_RINGBUF 10
#define MAX_CONNECTION 48
#define DAQ_NEVENT  3

#include <iostream> //cout
#include <string.h> //strcpy
//#include <string>//istringstream
#include <stdio.h>//sprintf
#include <unistd.h>//for sleep()
#include <fstream>//filestream(file I/O)
#include <sstream>//stringstream
#include <iomanip>//padding cout
#include <stdlib.h> //exit(1)

#include <pthread.h>
#include "RingBuffer.hpp"
#include "DAQtimer.hpp"
using namespace std;
int infreq = 0;
/****************************/
// struct definition
/****************************/
struct sRingBuffer{
  int sRBid;//RingBufferID
  int Cid;//Collector ID
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
    sRB[i].Cid = -1;
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

void sRBsetaddr(int sRBid, unsigned short shCid, char *szAddr, unsigned short shPort)
{
  if(sRB[sRBid].sRBid==sRBid)
  {
    sRB[sRBid].Cid=(int)shCid;
    strcpy(sRB[sRBid].szAddr, szAddr);
    sRB[sRBid].shPort=shPort;
  }
}


/********************************/
// Collector thread definition takecare! not same as Master.cpp
/********************************/
void *Collector_thread(void *arg)
{

  //receive buffer(From socket to ringbuffer)
  char tempbuf[976];
  
  cout<<"*** Collector_thread initialization ***"<<endl;
  //first RingBuffer
  sRingBuffer *srb[MAX_RINGBUF];
  srb[0]= (sRingBuffer*)arg;
  //CollectorID
  int Cid=srb[0]->Cid;
  //next RingBuffers
  cout<<"next RBs for Coll "<<Cid<<endl;
  sRingBuffer *srb_temp=srb[0]->next;
  int nServ=1;
  while(1)
  {
    if(srb_temp->sRBid==-1)break;
    cout<<"RB"<<srb_temp->sRBid<<": Cid is "<<srb_temp->Cid;
    if(srb_temp->Cid == Cid)
    {
      srb[nServ]=srb_temp;
      cout<<" matched."<<endl;
      nServ++;
    }
    else
    {
      cout<<endl;
    }
    srb_temp=srb_temp->next;
    //cout<<"srb["<<nServ<<"]->next :"<<srb_temp->next<<endl;
  }
  

  
  //instead of connection and read
  for(int i=0;i<nServ;i++)
    sprintf(tempbuf,"Hello World!Collector thread wrote %d",srb[i]->sRBid);
  
  //sleep(1);
  //  while(1)
  cout<<"aaaaaaaa"<<srb[0]->sRBid<<"nServ is "<< nServ<<endl;
  for(int i = 0; i< DAQ_NEVENT;i++)
  {
    for(int j=0;j<nServ;j++)
    {
      srb[j]->rb->write((void*)tempbuf);
      //sleep(1);
      cout << "Coll"<<srb[j]->sRBid <<" wrote :"<< tempbuf <<endl;
    }
  }
}

/********************************/
// Builder thread definition
/********************************/
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
      //cout << "Bld read from Coll"<<srb[i]->sRBid <<" :"<< tempbuf <<endl;
    }
    dt->readend();
    if(llMesReadBytes[nColl-1]>=ReadBytes)break;
  }
  dt->DAQend();
  dt->DAQsummary(infreq);
  //sleep(1);
}
/****************************/
// main definition
/****************************/
int main(int argc, char** argv)
{
  if(argc==2)infreq = atoi(argv[1]);
  cout << "Hello world!!init" <<endl;
  /******************************************/
  //  Reading Connection Configuration
  /******************************************/
  unsigned short shCid[MAX_CONNECTION]={0};
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
    iss >> shCid[nServ]>> szAddr[nServ] >> shPort[nServ];
    nServ++;
  }
  if(nServ>MAX_CONNECTION){
    printf("The number of connections excessed limit.");
    exit(1);
  }
  
  //Cid validation
  int maxCid=0;
  for(int i=0;i<nServ;i++)
  {
    if(maxCid<shCid[i])maxCid=shCid[i];
  }
  for(int i=0;i<maxCid;i++)
  {
    bool Cid_exist=false;
    for(int j=0;j<nServ;j++)
    {
      if(shCid[j]==i)
      {
        Cid_exist=true;
        break;
      }
    }
    if(!Cid_exist)
    {
      cout<<"error: Cid "<<i<<"is skipped."<<endl;
      exit(1);
    }
  }
  int nColl=maxCid+1;
  int firstRB[MAX_CONNECTION];
  //substitute the first connection(sRBid)
  for(int i=0;i<nColl;i++)
  {
    for(int j=0;j<nServ;j++)
    {
      if(shCid[j]==i)//if given CollID matches Cid
      {
        firstRB[i]=j;//Coll starts to seek from it
        break;
      }
    }
  }
  sRBinit();
  sRBcreate(nServ);
  for(int i=0;i<nServ;i++)
    sRBsetaddr(i,shCid[i],szAddr[i],shPort[i]);

  cout<<"****** Configuration of RinbBuffers are set ******"<<endl;
  cout<<nColl<<" Collectors will be created for "<<nServ<<" connections."<<endl;
  cout<<"RBid "<<"shCid "<<"     IP address     "<<" port "<<endl;
  for(int i=0;i<nServ;i++)
    cout<<"RB"<< setw(2) << setfill(' ')<<i<<"|"<<
    setw(6) << setfill(' ')<<sRB[i].Cid<<"|"<<
    setw(18) << setfill(' ')<<sRB[i].szAddr<<"|"<<
    setw(5) << setfill(' ')<<sRB[i].shPort<<endl;
  
  /******************************************/
  //  Multi-threaded process creation
  /******************************************/
  cout << "***  Thread create  ***"<<endl;
  pthread_t handle[nColl+1];
  for(int i=0;i<nColl;i++)
  {
    pthread_create(&handle[i],
                   NULL,
                   &Collector_thread,
                   &sRB[firstRB[i]]);
  }
  //sleep(1);
  pthread_create(&handle[nColl],
                 NULL,
                 &Builder_thread,
                 &sRB[0]);
  cout <<"Threads created. "<<
      DAQ_NEVENT << "events will be transferred each"<<endl;
  for(int i=0;i<nColl+1;i++)
    pthread_join(handle[i],NULL);
  
  cout << "Hello world!!end" <<endl;
  return 0;
}
