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
  char tempbuf[976];
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
  for (int i = 0; i<10; i++)
  {
    tcps->readSock(tempbuf,976);
    rb->write(tempbuf);
  }

}

void *Builder_thread(void *arg)
{
  unsigned long long llRead = 0;
  char outdata[976];
  LSTDAQ::RingBuffer *rb = (LSTDAQ::RingBuffer*)arg;
 while(1)
  // for (int i = 0; i<10; i++)
  {
    llRead +=rb->read(outdata);
    if(llRead>=9760)break;
    
  }


}


int main()
{
//  using namespace std;
  pthread_t handle[2];
  cout<<"comehere1"<<endl;
// 　pthread_mutex_t mutex;
  LSTDAQ::RingBuffer *rb = new LSTDAQ::RingBuffer();
  cout<<"comehere2"<<endl;
  //rb = new LSTDAQ::RingBuffer();
  cout<<"comehere3"<<endl;
  // pthread_mutex_init(&mutex, NULL);
  pthread_create(&handle[0],
                   NULL,
                   &Builder_thread,
                   (void*)rb);
  cout<<"comehere4"<<endl;
  pthread_create(&handle[1],
                   NULL,
                   &Collector_thread,
                   (void*)rb);
  cout<<"comehere5"<<endl;
  for(int i=0;i<2;i++)
    pthread_join(handle[i],NULL);


  //delete rb;



  std::cout<<"Hello world!"<<std::endl;
  return 0;
}

