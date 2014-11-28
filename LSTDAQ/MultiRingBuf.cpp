#include <iostream> //cout
#include <string.h> //strcpy
#include <stdio.h>//sprintf

#include "RingBuffer.hpp"
using namespace std;

struct RingBufArray{
  LSTDAQ::RingBuffer *rb[3] ;
};

void otherfunc(void *arg)
{
  char tempbuf[976];
  RingBufArray *rba = (RingBufArray*)arg;
  
  for(int i=0; i<3;i++)
    {
      (rba->rb[i])->read((void*)tempbuf[i]);
    }

  for(int i=0; i<3;i++)
    {
      cout << tempbuf[i] <<endl;
    }

}

int main()
{
  char tempbuf[976];
  char tempbuf2[3][976];
  // cout<<" Hello world!!" <<endl;
  // for(int i =0; i<3; i++)
  //   {

  //   }
  strcpy(tempbuf,"Hello world!!nagamenishokika");

  cout << tempbuf <<endl;
  LSTDAQ::RingBuffer *rb[3];
  for (int i =0; i< 3; i++)
    {
      rb[i] = new LSTDAQ::RingBuffer();
    }
  for(int i=0; i<3;i++)
    {
      sprintf(tempbuf,"Hello World!%d",i);
      rb[i]->write(tempbuf);      
    }
  // for(int i=0; i<3;i++)
  //   {
  //     rb[i]->read(tempbuf2[i]);
  //   }

  // for(int i=0; i<3;i++)
  //   {
  //     cout << tempbuf2[i] <<endl;
  //   }
  RingBufArray rba;
  for(int i=0;i<3;i++)
    rba.rb[i] = rb[i];
  otherfunc(&rba);
  return 0; 
}
