#include "RingBuffer.hpp"
#include <iostream>
#include <unistd.h>


namespace LSTDAQ{
  RingBuffer::RingBuffer() throw()
  {
    pthread_mutex_init(m_mutex,NULL);
    m_is_read=true;
  }
  RingBuffer::~RingBuffer() throw()
  {
    pthread_mutex_destroy(m_mutex);
  }
  RingBuffer &RingBuffer::operator = (const RingBuffer &rb) throw()
  {
    m_mutex = rb.m_mutex;
    return *this;
  }
  bool RingBuffer::open()
  {
  }
  bool RingBuffer::init()
  {
  }
  // unsigned int RingBuffer::write(unsigned int *)
  unsigned int RingBuffer::write(unsigned int *buf)
  {
    //mutex lock
    pthread_mutex_lock(m_mutex);
    while(!m_is_read)//==false)
    {
      pthread_mutex_unlock(m_mutex);
      sleep(1);
      pthread_mutex_lock(m_mutex);
    }
    memcpy(m_buffer,buf,128);
    m_is_read=false;
    //mutex unlock
    pthread_mutex_unlock(m_mutex);
    return sizeof(*buf);
  }
  unsigned int RingBuffer::read(unsigned int *buf)
  {
    //mutex lock
    pthread_mutex_lock(m_mutex);
    while(1)
    {
      if(!m_is_read)
      {
        sched_yield(); //スレッド切り替えチャンスを与える！！？
        memcpy(buf,m_buffer,128);
        m_is_read=true;
        pthread_mutex_unlock(m_mutex);
        break;
      }else{
        pthread_mutex_unlock(m_mutex);
        sleep(1);
      }
    }
  }

}
