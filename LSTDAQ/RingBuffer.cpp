#include "RingBuffer.hpp"
#include <iostream>
#include <unistd.h>

//yet read() and write() is to be modified
//nbytes should not be used
//While the EVENTSIZE should be variable
// for multiple connection.

namespace LSTDAQ{
  RingBuffer::RingBuffer() throw():m_Nm(1000)
  {
    //mutex initialization
    pthread_mutex_init(m_mutex,NULL);
    pthread_cond_init(m_cond,NULL);
    //info initialization
//    m_Nm=1000;
    m_Nw=0;
    m_Nr=0;
    m_Nmw=0;
    m_Nmr=0;
    //buffer initialization
//    for (int i=0; i<EVENTSIZE*Nm; i++) {
//      m_buffer++;
//    }
  }
  RingBuffer::~RingBuffer() throw()
  {
    pthread_mutex_destroy(m_mutex);
  }
  RingBuffer &RingBuffer::operator = (const RingBuffer &rb) throw()
  {
    m_mutex = rb.m_mutex;
    m_cond = rb.m_cond;
    return *this;
  }
  bool RingBuffer::open()
  {
  }
  bool RingBuffer::init()
  {
  }
  unsigned int RingBuffer::write(void *buf)
  {
    //mutex lock
    pthread_mutex_lock(m_mutex);
    //wait to prevent from overwriting
    if ( m_Nw > m_Nr && m_Nmw == m_Nmr) {
      pthread_cond_wait(m_cond, m_mutex);
    }
    memcpy(m_buffer + EVENTSIZE * m_Nmw + 1,buf , EVENTSIZE);
    m_Nw++;
    m_Nmw++;
    if (m_Nmw == m_Nm) {
      m_Nmw=0;
    }
    //mutex unlock
    pthread_mutex_unlock(m_mutex);
    return EVENTSIZE;
  }
  unsigned int RingBuffer::read(void *buf)
  {
    //mutex lock
    pthread_mutex_lock(m_mutex);
    //wait to prevent from overreading
    if (m_Nr==m_Nw) {
      pthread_cond_wait(m_cond, m_mutex);
    }
    memcpy(buf,m_buffer + EVENTSIZE * m_Nmr + 1,EVENTSIZE);
    m_Nr++;
    m_Nmr++;
    if (m_Nmr == m_Nm) {
      m_Nmr=0;
    }
    //mutex unlock
    pthread_mutex_unlock(m_mutex);
    return EVENTSIZE;
  }

}
