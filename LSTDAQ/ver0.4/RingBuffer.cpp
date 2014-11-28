#include "RingBuffer.hpp"
#include <iostream>
#include <unistd.h>
#include <string.h>//memcpy
#include <stdlib.h>//malloc

//yet read() and write() is to be modified
//nbytes should not be used
//While the EVENTSIZE should be variable
// for multiple connection.

namespace LSTDAQ{
  RingBuffer::RingBuffer() throw():m_Nm(1000)
  {
    //mutex initialization
   std::cout<<"constructor 1"<<std::endl;
    m_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(m_mutex,NULL);
   std::cout<<"constructor 2"<<std::endl;
    m_cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
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
    std::cout<<"hello write-->";//<<std::endl; 
    if ( m_Nw > m_Nr && m_Nmw == m_Nmr) {
      std::cout<<"write wait-->"<<std::endl; 
     pthread_cond_wait(m_cond, m_mutex);

    }
    memcpy(m_buffer + EVENTSIZE * m_Nmw,buf , EVENTSIZE);
    m_Nw++;
    m_Nmw++;
    if (m_Nmw == m_Nm) {
      m_Nmw=0;
    }
    std::cout<<"write end "<<m_Nw<<std::endl;
    //mutex unlock
    pthread_cond_signal(m_cond);
    pthread_mutex_unlock(m_mutex);
    return EVENTSIZE;
  }
  unsigned int RingBuffer::read(void *buf)
  {
    sleep(2);
    //mutex lock
    pthread_mutex_lock(m_mutex);
    std::cout<<"hello read-->";//<<std::endl;
    //wait to prevent from overreading
    if (m_Nr==m_Nw) {
      std::cout<<"read wait-->";//<<std::endl;
      pthread_cond_wait(m_cond, m_mutex);
    }
    memcpy(buf,m_buffer + EVENTSIZE * m_Nmr,EVENTSIZE);
    m_Nr++;
    m_Nmr++;
    std::cout<<"read end"<<m_Nr<<std::endl;
    if (m_Nmr == m_Nm) {
      m_Nmr=0;
    }
    //mutex unlock
    pthread_cond_signal(m_cond);
    pthread_mutex_unlock(m_mutex);
    return EVENTSIZE;
  }

}
