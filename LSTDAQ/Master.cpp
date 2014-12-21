/******************************/
// LSTDAQ ver2.0
//  Kazuma Ishio
//  Modified on 2014/12/21
/******************************/



#include <iostream> //cout
#include <string.h> //strcpy
//#include <string>
#include <stdio.h>  //sprintf
#include <unistd.h> //for sleep()andusleep()
#include <fstream>  //filestream(file I/O)
#include <sstream>  //stringstream
#include <iomanip>  //padding cout
#include <stdlib.h> //exit(1)

#include <pthread.h>
#include "RingBuffer.hpp"
#include "TCPClientSocket.hpp"
#include "DAQtimer.hpp"
#include "Config.hpp"


#define DAQ_NEVENT  100000
int infreq;
int Ndaq;
bool datacreate;

//variables for start synchronizer
pthread_mutex_t mutex_initLock  =PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_allend      =PTHREAD_COND_INITIALIZER;
int initEnd;
//variable of the number of CPU in the system on which this runs
int Ncpu;

using namespace std;
/****************************/
// print usage
/****************************/
void usage(char *argv)
{
  printf("usage: %s <infreq[Hz]> <Ndaq[events]> <datacreate> \n",argv);
  printf("<Ndaq>       -- optional.default value is %d\n",DAQ_NEVENT);
  printf("<datacreate> -- 1:create data. 0 or no specification: don't create data.\n"); 
  exit(0);
}

/****************************/
// struct definition
/****************************/
struct sRingBuffer{
  int sRBid;            //RingBufferID
  int Cid;              //Collector ID
  char szAddr[18];      //="192.168.10.1"
  unsigned short shPort;//= 24;
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
    // cout <<sRB[i].next<<endl;
  }
}

void sRBcreate(int nServ)
{
  for(int i=0; i<nServ; i++)
  {
    sRB[i].sRBid = i;
    sRB[i].rb = new LSTDAQ::RingBuffer();
    sRB[i].next=&sRB[i+1];
    // cout <<sRB[i].next<<endl;
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

int getMaxCid()
{
  int maxCid=0;
  for(int i=0;i<MAX_RINGBUF;i++)
  {
    if(maxCid<sRB[i].Cid)maxCid=sRB[i].Cid;
  }
  return maxCid;
  
}

void sRBdelete(int nServ)
{
  for(int i=0; i<nServ; i++)
  {
    delete sRB[i].rb;
  }
}


/********************************/
// Collector thread definition takecare! not same as MultiRingBuf.cpp
/********************************/
void *Collector_thread(void *arg)
{
  
  //receive buffer(From socket to ringbuffer)
  char tempbuf[EVENTSIZE];
  
  // cout<<"*** Collector_thread initialization ***"<<endl;
  /******************************************/
  //  First RB and Collector ID
  /******************************************/
  //first RingBuffer
  sRingBuffer *srb[MAX_CONNECTION];
  srb[0]= (sRingBuffer*)arg;
  //CollectorID
  int Cid=srb[0]->Cid;
  
  
  /******************************************/
  //  CPU Specification
  /******************************************/
  int cpuid;
  cpuid = Cid%Ncpu;
#ifdef __CPU_ZERO
  cpu_set_t mask;
  __CPU_ZERO(&mask);
  __CPU_SET(Ncpu,&mask);
  if(sched_setaffinity(0,sizeof(mask), &mask) == -1)
    printf("WARNING: failed to set CPU affinity... (cpuid=%d)\n",cpuid);
#endif
  
  
  /******************************************/
  //  Next RBs
  /******************************************/
  //next RingBuffers
  // cout<<"next RBs for Coll "<<Cid<<endl;
  sRingBuffer *srb_temp=srb[0]->next;
  int nServ=1;
  while(1)
  {
    if(srb_temp->sRBid==-1)break;
    // cout<<"RB"<<srb_temp->sRBid<<": Cid is "<<srb_temp->Cid;
    if(srb_temp->Cid == Cid)
    {
      srb[nServ]=srb_temp;
      // cout<<" matched."<<endl;
      nServ++;
    }
    // else
    // {
    //   cout<<endl;
    // }
    srb_temp=srb_temp->next;
    //cout<<"srb["<<nServ<<"]->next :"<<srb_temp->next<<endl;
  }
  // cout<< "*** RingBufferList owned by Collector"<<srb[0]->Cid<<endl;
  // for(int i=0;i<nServ;i++)
  // {
  //   cout<<"RB"<<srb[i]->sRBid<<endl;
  // }
  // cout<<" :nServ is "<< nServ<<endl;
  
  /******************************************/
  // Connection Initialization
  /******************************************/
  cout<<"*** connection initialization ***"<<endl;
  unsigned long lConnected = 0;
  LSTDAQ::LIB::TCPClientSocket *tcps[MAX_CONNECTION];
  int sock[MAX_CONNECTION];
  for(int i=0;i<nServ;i++)
  {
    tcps[i] = new LSTDAQ::LIB::TCPClientSocket();
    if((tcps[i]->connectTcp(srb[i]->szAddr,
                            srb[i]->shPort,
                            lConnected)
        )<0)
    {
      exit(1);
    }
    sock[i] = tcps[i]->getSock();
  }
  int maxfd=sock[0];
  fd_set fds, readfds;
  FD_ZERO(&readfds);
  for(int i=0;i<nServ;i++)FD_SET(sock[i], &readfds);
  for(int i=1;i<nServ;i++)
  {
    if(sock[i]>maxfd)maxfd=sock[i];
  }
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 10000;
  
  /******************************************/
  //  Start Synchronization
  /******************************************/
  // cout<<"*** CollInit end ***"<<endl;
  initEnd++;
  pthread_mutex_lock(&mutex_initLock);
  pthread_cond_wait(&cond_allend,&mutex_initLock);
  pthread_mutex_unlock(&mutex_initLock);
  
  /******************************************/
  //  Start to Read From sock
  //         and Write on RB
  /******************************************/
  unsigned long long daqsize=Ndaq * EVENTSIZE;
  cout<<"*** Collector_thread starts to read ***"<<endl;
  // cout<<daqsize<<" will be read"<<endl;
  unsigned long long llReadBytes[MAX_CONNECTION]={0};
  int nRdBytes;
  bool bReadEnd[MAX_CONNECTION]={false};
  int ReadEnd=0;
  for(;;)
  {
    memcpy(&fds,&readfds,sizeof(fd_set));
    select(maxfd+1, &fds, NULL, NULL,&tv);
    for(int i=0;i<nServ;i++)
    {
      if( FD_ISSET(sock[i], &fds) )
      {
        nRdBytes=(tcps[i]->readSock(tempbuf,EVENTSIZE));
        //if(nRdBytes != EVENTSIZE) is usual
        llReadBytes[i]+=nRdBytes;
        // cout<<"RB"<<srb[i]->sRBid<<"read"<<nRdBytes<<endl;
        // cout<<i<<"  "<<nRdBytes<<"  "<<llReadBytes[i]<<endl;
        // // cout<<tempbuf<<endl;
        
        if((srb[i]->rb->write(tempbuf,nRdBytes))==-1)
        {
          cout<<"RB"<<srb[i]->sRBid<<":W wait exceeded"<<endl;
        }
        //cout<<"connection"<<i<<bReadEnd[i]<<endl;
        if(llReadBytes[i]>=daqsize && !bReadEnd[i])
        {
          ReadEnd++;
          bReadEnd[i]=true;
          // cout<<"RB"<<i<<"read end"<<endl;
        }
      }
      //usleep(1000000);
      //cout << "Coll"<<srb[j]->sRBid <<" wrote :"<< tempbuf <<endl;
    }
    if (ReadEnd==nServ)break;
  }
  //sleep(3);
  cout << "Coll"<<srb[0]->Cid <<" thread end"<<endl;
}

/********************************/
// Builder thread definition
/********************************/
void *Builder_thread(void *arg)
{
  //basic preparation
  sRingBuffer *srb[MAX_CONNECTION];
  int offset;
  srb[0]= (sRingBuffer*)arg;
  char tempbuf[EVENTSIZE*MAX_CONNECTION];

  

  //cout<<"*** Builder_thread initialization ***"<<endl;
  int nRB =0;
  while(1)
  {
    // cout<<"srb["<<nRB<<"] :"<<srb[nRB]->next<<endl;
    if(srb[nRB]->next==0||nRB==MAX_RINGBUF)break;
    srb[nRB+1]=srb[nRB]->next;
    nRB++;
  }
  
  int nColl = 1+ getMaxCid();
  
  //*********** CPU Specification    *************//
  int cpuid;
  cpuid = nColl%Ncpu;
#ifdef __CPU_ZERO
  cpu_set_t mask;
  __CPU_ZERO(&mask);
  __CPU_SET(Ncpu,&mask);
  if(sched_setaffinity(0,sizeof(mask), &mask) == -1)
    printf("WARNING: failed to set CPU affinity... (cpuid=%d)\n",cpuid);
#endif
  
  //*********** Output File Creation *************//
  char buf[128];
  sprintf(buf,"infreq%d_nColl%d_nRB%d.dat"
          ,infreq
          ,nColl
          ,nRB);
  FILE *fp_data;
  fp_data = fopen(buf,"w");
  int dataLength = EVENTSIZE*nRB;
  
  //*********** Start Synchronization *************//
  while(initEnd<nColl);
  // cout<<"Bld Confirmed all"<<endl;
  cout<<"*** Builder_thread starts to read ***"<<endl;
  cout<<Ndaq<<"events from "<<nRB<<"RBs"<<endl;
  pthread_mutex_lock(&mutex_initLock);
  pthread_cond_broadcast(&cond_allend);
  pthread_mutex_unlock(&mutex_initLock);
  
  //*********** Read Data From RingBuffer without build *************//
 // bool bReadEnd[MAX_CONNECTION]={false};
 // int ReadEnd=0;
 // unsigned long Nread[MAX_CONNECTION]={0};
 // LSTDAQ::DAQtimer *dt=new LSTDAQ::DAQtimer(nRB);
 // dt->DAQstart();
 // while(1)
 // {
 //   //cout<<"@@"<<ReadEnd<<endl;
 //   offset = 0;
 //   for(int i =0;i<nRB;i++)
 //   {
 //     //Nread[i] =(srb[i]->rb->read(tempbuf));
 //     Nread[i]=srb[i]->rb->read(tempbuf);
 //     // cout << "Bld read from Coll"<<srb[i]->sRBid <<" :"<< tempbuf <<endl;
 //     //cout<<i<<":"<<Nread[i];//<<endl;
 //     if(Nread[i]>=Ndaq && !bReadEnd[i])
 //     {
 //       ReadEnd++;
 //       bReadEnd[i]=true;
 //     }
 //     //cout<<"ReadEnd"<<ReadEnd<<endl;
 //   }
 //   dt->readend();
 //   if (ReadEnd==nRB)break;
 // }
//  
//  dt->DAQend();
//  dt->DAQsummary(infreq,Ndaq,nRB,nColl);
//  fclose(fp_data);
//  cout << "Builder thread end."<< Nread[0]<<"data was read."<<endl;


  //*********** Read Data From RingBuffer with build *************//
  unsigned long Nread=0;
  unsigned int *p_Ntrg[MAX_CONNECTION];
  unsigned int *p_Nevt[MAX_CONNECTION];
  unsigned long Ntrg[MAX_CONNECTION]={0};
  unsigned long Nevt[MAX_CONNECTION]={0};
  unsigned int *pa;
  unsigned int a;
  char header[3];
  LSTDAQ::DAQtimer *dt=new LSTDAQ::DAQtimer(nRB);
  dt->DAQstart();
  while(Nread<Ndaq)
  {
    offset=0;
    Nread++;
    for(int i=0;i<nRB;i++)
    {
      while(Nread!=srb[i]->rb->read(&tempbuf[offset]))
      {
        continue;
      }
        p_Nevt[i]=(unsigned int*)&tempbuf[offset+HEADERLEN+IPADDRLEN];
        p_Ntrg[i]=(unsigned int*)&tempbuf[offset+HEADERLEN+IPADDRLEN+EVTNOLEN];
	// printf("%c%c ",tempbuf[offset],tempbuf[offset+1]);
	// printf("%d ",*p_Nevt[i]);
        //cout<<"nowRB"<<i;//<<endl;
	
	Nevt[i]=(unsigned long)*p_Nevt[i];
	Ntrg[i]=(unsigned long)*p_Ntrg[i];
	if(Nread+DAQ_NEVENT<Ntrg[i])
	{
	  dt->DAQerrend(i,infreq,Nread,nRB,nColl,Ntrg,Nevt);
	  exit(1);
	}
	// printf("%d ",Nevt[i]);
	// printf("%d ",Ntrg[i]);

      offset+=EVENTSIZE;
    }
    dt->readend();
    //fwrite;
    if(datacreate==true)
      fwrite(&tempbuf,dataLength,1,fp_data);
    // usleep(300000);
  }
  dt->DAQend();
  dt->DAQsummary(infreq,Nread,nRB,nColl,Ntrg,Nevt);
  fclose(fp_data);
  cout << "Builder thread end."<< Nread<<"data was read."<<endl;
  //sleep(1);
}


/****************************/
// main definition
/****************************/
int main(int argc, char** argv)
{
  /******************************************/
  //  Set parameters from args
  /******************************************/
  infreq=0;
  Ndaq=DAQ_NEVENT;
  datacreate=false;
  if(argv[1][0]=='h'||argc >4)
    usage(argv[0]);
  if(argc >= 2)
    infreq = atoi(argv[1]);
  if(argc >= 3)
    Ndaq = atoi(argv[2]);
  if(argc == 4)
  {
    switch (atoi(argv[3]))
    {
    case 1:
      cout<<"NOTE:data will be created."<<endl;
      datacreate = true;
      break;
    case 0:
      datacreate = false;
      break;
    default:
      usage(argv[0]);
    }
  }


  cout << "***LSTDAQ starts***" <<endl;
  
  /******************************************/
  //  CPU Inspection
  /******************************************/
  Ncpu = sysconf(_SC_NPROCESSORS_CONF);
  cout<<"This machine has"<<Ncpu<<" cpus"<<endl;
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
  // cout << "***  Thread create  ***"<<endl;
  pthread_t handle[nColl+1];
  for(int i=0;i<nColl;i++)
  {
    pthread_create(&handle[i],
                   NULL,
                   &Collector_thread,
                   &sRB[firstRB[i]]);
    // sleep(1);
  }
  pthread_create(&handle[nColl],
                 NULL,
                 &Builder_thread,
                 &sRB[0]);
  // cout <<"Threads created. "<<
  // Ndaq << "events will be transferred each"<<endl;
  for(int i=0;i<nColl+1;i++)
    pthread_join(handle[i],NULL);
  
  cout << "LSTDAQ end" <<endl;
  return 0;
}
