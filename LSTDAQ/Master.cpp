#include <stdio.h>
#include <pthread.h>

#include <iostream>
#include <string>
#include <unistd.h>//for sleep()
#include "RingBuffer.hpp"
#include "TCPClientSocket.hpp"


using namespace std;

void *Collector_thread(void *arg)
{
  //receive buffer(From socket to ringbuffer)
  unsigned char tempbuf[976];
  //ring buffer(From receive buffer to Builder)
  LSTDAQ::RingBuffer *rb = (LSTDAQ::RingBuffer*)arg;

  //connection
  char szAddr[18]="192.168.10.1";
  unsigned short shPort = 24;
  unsigned long lConnected = 0;
  LSTDAQ::LIB::TCPClientSocket *tcps;
  tcps = new LSTDAQ::LIB::TCPClientSocket();
  tcps->connectTcp(szAddr,shPort,lConnected);

  
//  while(1)
  for (int i = 0; i<100; i++)
  {
    tcps->readSock(tempbuf,976);
    rb->write((unsigned int *)tempbuf);
  }

}

void *Builder_thread(void *arg)
{
  char outdata[976];
  LSTDAQ::RingBuffer *rb = (LSTDAQ::RingBuffer*)arg;
//  while(1)
  for (int i = 0; i<100; i++)
  {
    rb->read((unsigned int*)outdata);
    //cout<<outdata<<endl;
  }


}


int main()
{
  
//  using namespace std;
　pthread_t handle[2];
// 　pthread_mutex_t mutex;
  LSTDAQ::RingBuffer *rb;
  // pthread_mutex_init(&mutex, NULL);
  pthread_create(&handle[0],
                   NULL,
                   &Builder_thread,
                   (void*)rb);
  pthread_create(&handle[1],
                   NULL,
                   &Collector_thread,
                   (void*)rb);
  for(int i=0;i<2;i++)
    pthread_join(handle[i],NULL);


  delete rb;



  std::cout<<"Hello world!"<<std::endl;
  return 0;
}

