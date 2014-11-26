#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H
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
    unsigned int write(unsigned int *buf);
    unsigned int read(unsigned int *buf);

    //
  private:
    char m_buffer[128];
    pthread_mutex_t *m_mutex;
    bool m_is_read;
    
  };
}


#endif
