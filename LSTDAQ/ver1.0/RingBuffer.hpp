#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H
#define EVENTSIZE 976
#define RINGBUFSIZE 1000
#include <pthread.h>
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
    unsigned int write( char *buf,unsigned int wbytes);
    int read(char *buf);

    //
  private:
    pthread_mutex_t *m_mutex;
    pthread_cond_t *m_cond;
    
    //buffer
    const unsigned int m_Nm;
    unsigned char m_buffer[EVENTSIZE*RINGBUFSIZE];
    unsigned int m_bufSizeByte;
    unsigned int m_offset;
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
