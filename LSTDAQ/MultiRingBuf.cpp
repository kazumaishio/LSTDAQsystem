#include <iostream> //cout
#include <string.h> //strcpy
#include <stdio.h>//sprintf

#include "RingBuffer.hpp"
using namespace std;

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
  for(int i=0; i<3;i++)
    {
      rb[i]->read(tempbuf2[i]);
    }

  for(int i=0; i<3;i++)
    {
      cout << tempbuf2[i] <<endl;
    }
  return 0; 
}
