#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H

#ifndef EVENTSIZE 
#define EVENTSIZE 976
#endif

#define RINGBUFSIZE 50000
#define TIMETOWAIT  0
#define TIMETOWAIT_USEC  1000
#include <pthread.h>
#include "Config.hpp"
namespace LSTDAQ{
  class RingBuffer
  {
  public:
    RingBuffer() throw();
    virtual ~RingBuffer() throw();

    RingBuffer &operator = (const RingBuffer &)throw();
    //methods
    bool open();
    bool init();
    int write( char *buf,unsigned int wbytes);
    int read(char *buf);

    //getter methods
    unsigned long getNw() throw();
    unsigned long getNr() throw();
    
  private:
    pthread_mutex_t *m_mutex;
    pthread_cond_t *m_cond;
    
    //time to wait (on write function)
    struct timespec m_tsWait;
    
    //buffer
    const unsigned int m_Nm;
    unsigned char m_buffer[EVENTSIZE*RINGBUFSIZE];
    unsigned int m_bufSizeByte;
    unsigned int m_woffset;
    unsigned int m_roffset;
    unsigned int m_remain;
    unsigned int m_wbytes;//written to the memory
    //total history
    unsigned long m_Nw;  //events written to the memory
    unsigned long m_Nr;  //read from  the memory
    //the position on memory
    unsigned int  m_Nmw;  //written to the memory
    unsigned int  m_Nmr;  //read from  the memory
  };
}


#endif
