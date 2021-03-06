#include <iostream> //cout
#include <string.h> //strcpy
#include <stdio.h>//sprintf
#include <unistd.h>//for sleep()
#include <pthread.h>
#include "RingBuffer.hpp"
using namespace std;

#define MAX_RINGBUF 10
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
//void sRBadd();

void sRBinit()
{
  for(int i=0; i<MAX_RINGBUF; i++)
  {
    sRB[i].sRBid = -1;
    sRB[i].next = 0;
    cout <<sRB[i].next<<endl;
  }
}

void sRBcreate(int Nconn)
{
  for(int i=0; i<Nconn; i++)
  {
    sRB[i].sRBid = i;
    sRB[i].rb = new LSTDAQ::RingBuffer();
    sRB[i].next=&sRB[i+1];
        cout <<sRB[i].next<<endl;
  }
}

//void sRBadd(LSTDAQ::RingBuffer* rb)
//{
//  sRBins(rb,9999);
//}
//void sRBinsert(LSTDAQ::RingBuffer* rb)
//{
//  sRingBuffer *freap, *insp;
//  int last;
//  freep =
//}


/****************************/
// thread definition
/****************************/

void *Collector_thread(void *arg)
{
  char tempbuf[976];
  sRingBuffer *srb = (sRingBuffer*)arg;

  sprintf(tempbuf,"Hello World!Collector thread wrote %d",srb->sRBid);
  srb->rb->write((void*)tempbuf);
  //sleep(1);
  cout << "Coll"<<srb->sRBid <<" wrote :"<< tempbuf <<endl;
  
}

void *Builder_thread(void *arg)
{
  char tempbuf[976];
  sRingBuffer *srb[MAX_RINGBUF];
  srb[0]= (sRingBuffer*)arg;
  int Nconn =0;
  while(1)
  {
    cout<<"aaa"<<endl;
    cout<<"srb["<<Nconn<<"] :"<<srb[Nconn]->next<<endl;
    if(srb[Nconn]->next==0||Nconn==MAX_RINGBUF)break;
    srb[Nconn+1]=srb[Nconn]->next;
    Nconn++;
  }
  //read two times each.
  // one is written by main func
  // the other is written by Collector_thread
  for(int j=0;j<2;j++)
  {
    for(int i =0;i<Nconn;i++)
    {
      srb[i]->rb->read((void*)tempbuf);
      cout << "Bld read from Coll"<<srb[i]->sRBid <<" :"<< tempbuf <<endl;
    }
  }
  //sleep(1);

  
}
/****************************/
// main definition
/****************************/

int main()
{

  char tempbuf[976];
  char tempbuf2[3][976];
  strcpy(tempbuf,"Hello world!!init");
  cout << tempbuf <<endl;
  //Reads Connection.conf
  int Nconn=4;
  sRBinit();
  sRBcreate(Nconn);
  
  //struct test
  //giving struct to another function which only receives void* arg
  cout << "*** Object transfer test ***"<<endl;
  for(int i=0; i<Nconn;i++)
    {
      sprintf(tempbuf,"Hello World!main thread wrote. %d",i);
      sRB[i].rb->write(tempbuf);
    }
  //Multi-threaded process creation
  pthread_t handle[Nconn+1];
  for(int i=0;i<Nconn;i++)
  {
    pthread_create(&handle[i],
                   NULL,
                   &Collector_thread,
                   &sRB[i]);
//    pthread_create(&handle[i],
//                    NULL,
//                    &Builder_thread,
//                    &sRB[i]);
//
  }
  sleep(1);
  pthread_create(&handle[Nconn],
                 NULL,
                 &Builder_thread,
                 &sRB[0]);
  for(int i=0;i<Nconn+1;i++)
    pthread_join(handle[i],NULL);
  
  strcpy(tempbuf,"Hello world!!end");
  cout << tempbuf <<endl;
  return 0;
}
