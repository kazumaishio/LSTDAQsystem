#include <iostream> //cout
#include <string.h> //strcpy
#include <stdio.h>//sprintf

#include "RingBuffer.hpp"
using namespace std;

/****************************/
// struct definition
/****************************/
struct sRingBuffer{
  int sRBid;
  LSTDAQ::RingBuffer* rb ;
  sRingBuffer* next;
};

sRingBuffer sRB[10];
//void sRBinit();
//void sRBadd();

void sRBinit()
{
  for(int i=0; i<10; i++)
  {
    sRB[i].sRBid = -1;
    sRB[i].next = 0;
  }
}
//void sRBadd(LSTDAQ::RingBuffer* rb)
//{
//  sRBins(rb,9999);
//}
//void sRBins(LSTDAQ::RingBuffer* rb)
//{
//  sRingBuffer *freap, *insp;
//  int last;
//  freep =
//}

/****************************/
// thread definition
/****************************/

void otherfunc(void *arg)
{
  char tempbuf[976];
  sRingBuffer *srb = (sRingBuffer*)arg;
  
  //for(int i=0; i<3;i++)
    {
      srb->rb->read((void*)tempbuf);
    }

  //for(int i=0; i<3;i++)
    {
      cout << tempbuf <<endl;
    }

}

/****************************/
// main definition
/****************************/

int main()
{
  sRBinit();
  char tempbuf[976];
  char tempbuf2[3][976];
  strcpy(tempbuf,"Hello world!!init");
  cout << tempbuf <<endl;
  
  //object array test.
  LSTDAQ::RingBuffer *rb[3];
  for(int i=0;i<3;i++)rb[i]= new LSTDAQ::RingBuffer();
   for(int i=0; i<3;i++)
     {
       sprintf(tempbuf,"Array of object test  : Hello World!%d",i);
       rb[i]->write(tempbuf);
     }
  
   for(int i=0; i<3;i++)
     {
       rb[i]->read(tempbuf2[i]);
       cout << tempbuf2[i] <<endl;
     }
  //struct test
  //giving struct to another function which only receives void* arg
  cout << "*** Object transfer test ***"<<endl;
  for (int i =0; i< 3; i++)
    {
      sRB[i].rb = new LSTDAQ::RingBuffer();
    }
  for(int i=0; i<3;i++)
    {
      sprintf(tempbuf,"Hello World!%d",i);
      sRB[i].rb->write(tempbuf);
//      sRBadd(rb[i]);
//      sRB[i].rb = new LSTDAQ::RingBuffer();
      otherfunc(&sRB[i]);
    }
  //RingBufArray rba;

  return 0; 
}
