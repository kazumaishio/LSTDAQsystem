#include <stdio.h>
#include <pthread.h>

#include <iostream>
#include <string>
#include <unistd.h>//for sleep()
#include "RingBuffer.hpp"

//unsigned int __g_buff[128];//receive buffer
using namespace std;

void *Collector_thread(void *arg)
{
  LSTDAQ::RingBuffer *rb = (LSTDAQ::RingBuffer*)arg;
while(1)
{

  char tempdata[128];
  cin>>tempdata;
cin.clear();

  int outcount;
  rb->write((unsigned int *)tempdata);
}

}

void *Builder_thread(void *arg)
{
  char outdata[128];
  LSTDAQ::RingBuffer *rb = (LSTDAQ::RingBuffer*)arg;
while(1)
{

  rb->read((unsigned int*)outdata);
  cout<<outdata<<endl;
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

