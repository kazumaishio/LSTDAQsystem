#include "RingBuffer.hpp"
#include <iostream>
#include <unistd.h>
#include <string.h>//memcpy
#include <stdlib.h>//malloc
#include <stdlib.h> //exit(1)

//EVENTSIZE should be variable
// for multiple connection.
//m_Nm and EVENTSIZE should be defined more explicitly.

namespace LSTDAQ{
  RingBuffer::RingBuffer() throw():m_Nm(RINGBUFSIZE),m_bufSizeByte(RINGBUFSIZE*EVENTSIZE)
  {
    //mutex initialization
    m_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(m_mutex,NULL);
    m_cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    pthread_cond_init(m_cond,NULL);
    //info initialization
    m_offset=0;
    m_Nw=0;
    m_Nr=0;
    m_Nmw=0;
    m_Nmr=0;
    std::cout<<"constructor succeed"<<std::endl;
    std::cout<<"m_bufSizeByte"<<m_bufSizeByte<<std::endl;
    std::cout<<"RINGBUFSIZE"<<RINGBUFSIZE<<std::endl;
    std::cout<<"EVENTSIZE"<<EVENTSIZE<<std::endl;
    //buffer initialization
    /*needed??*/
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
  unsigned int RingBuffer::write( char *buf,unsigned int wbytes)
  {
    unsigned int retval;
    //****** mutex lock ******
    pthread_mutex_lock(m_mutex);
    //std::cout<<"hello write-->";//<<std::endl;
    //****** prevent from overwriting ******
    //std::cout<<"W wbytes"<<wbytes<<"m_wbytes"<<m_wbytes;
    if( m_Nw > m_Nr +RINGBUFSIZE -2)
    {
      std::cout<<"W m_Nw = "<<m_Nw<<",m_Nmw = "<<m_Nmw<<std::endl;
      std::cout<< " m_Nr = "<<m_Nr<<",m_Nmr = "<<m_Nmr<<std::endl;
      std::cout<<"write wait-->"<<std::endl;
      pthread_cond_wait(m_cond, m_mutex);
    }
    
    //****** write to RingBuffer ******
    if(m_wbytes+wbytes<EVENTSIZE)
    {
      memcpy(m_buffer+m_offset, buf, wbytes);
      m_offset +=wbytes;
      m_wbytes +=wbytes;
      //std::cout<<"rb1";
    }
    else
    {
      if(m_offset+wbytes>m_bufSizeByte)
      {
        m_remain = m_offset + wbytes - m_bufSizeByte;
        memcpy(m_buffer + m_offset  ,buf            ,wbytes - m_remain);
        m_offset = wbytes-m_remain;
        memcpy(m_buffer             ,buf + m_offset ,m_remain);
        m_offset = m_remain;
        m_wbytes = m_remain;
        m_Nw++;
        m_Nmw=0;
        //std::cout<<"rb2";
      }
      else
      {
        memcpy(m_buffer + m_offset ,buf ,wbytes);
        m_offset+=wbytes;
        m_wbytes =m_wbytes + wbytes -EVENTSIZE;
        m_Nw++;
        m_Nmw++;
        if (m_Nmw == RINGBUFSIZE) {
          m_Nmw=0;
          m_offset=0;
        }
        //std::cout<<"rb3";
      }
    }
    retval= m_Nw;
    // //std::cout<<"write end "<<std::endl;
    // std::cout<<"W m_Nw = "<<m_Nw<<",m_Nmw = "<<m_Nmw<<std::endl;
    //****** mutex unlock ******
    pthread_cond_signal(m_cond);
    pthread_mutex_unlock(m_mutex);
    return retval;
  }
  
  int RingBuffer::read(char *buf)
  {
    int retval;
    //sleep(2);
    //std::cout<<"hello read-->";//<<std::endl;
    //****** prevent from overreading ******
    if (m_Nr==m_Nw)
    {
      retval=m_Nr;
      return retval;
    }
    //****** read from RingBuffer ******
    else if(m_Nr<m_Nw)
    {
      pthread_mutex_lock(m_mutex);
      memcpy(buf,m_buffer + EVENTSIZE * m_Nmr,EVENTSIZE);
      m_Nr++;
      m_Nmr++;
      if (m_Nmr == RINGBUFSIZE)
        m_Nmr=0;
      // //std::cout<<"read end"<<std::endl;//
      // std::cout<< "R m_Nr = "<<m_Nr<<",m_Nmr = "<<m_Nmr<<std::endl;
      retval=m_Nr;
      //****** mutex unlock ******
      pthread_cond_signal(m_cond);
      pthread_mutex_unlock(m_mutex);
      return retval;
    }
    else
    {
      exit(1);
    }
  }
}
