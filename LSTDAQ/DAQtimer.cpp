//****************************************************
// DAQ timer
//****************************************************
#include <time.h>     //for measuring time
#include <sys/time.h> //for making filename
#include "DAQtimer.hpp"
#include "RingBuffer.hpp" //EVENTSIZE
#include <errno.h>
#include <ctime>
#include <unistd.h>   //for inspecting directory
#include <sys/stat.h> //for making directory
#include <iostream>
#include <iomanip> //for padding

#include <fstream>
#include <sstream>
///////////////////////////////////////////////////////////////////////////////////////////
// clock_gettime(CLOCK_REALTIME) is not available on Mac OSX.
// instead, this is imported from https://gist.github.com/jbenet/1087739
///////////////////////////////////////////////////////////////////////////////////////////
//****************************************************
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

void current_utc_time(struct timespec *ts) {
  
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  ts->tv_sec = mts.tv_sec;
  ts->tv_nsec = mts.tv_nsec;
#else
  clock_gettime(CLOCK_REALTIME, ts);
#endif
}
//****************************************************
namespace LSTDAQ{
  DAQtimer::DAQtimer(int nServ) throw()
  {
    m_nServ = nServ;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    for(int i=0;i<1000;i++)
      lltime_diff[i]=0;
  }
  DAQtimer::~DAQtimer() throw()
  {
  }
  ///////////////////////////////////////////////////////////////////////////////////////////
  // time calc
  ///////////////////////////////////////////////////////////////////////////////////////////
  //#define TIME_SEC2NSEC	1000000000 //moved to .hpp file
  unsigned long long DAQtimer::GetRealTimeInterval(const  struct timespec *pFrom, const struct timespec *pTo)
  {
    unsigned long long  llStart = (unsigned long long )((unsigned long long )pFrom->tv_sec*TIME_SEC2NSEC + (unsigned long long )pFrom->tv_nsec);
    unsigned long long  llEnd = (unsigned long long )((unsigned long long )pTo->tv_sec*TIME_SEC2NSEC + (unsigned long long )pTo->tv_nsec);
    return( (llEnd - llStart)/1000 );
  }
  ///////////////////////////////////////////////////////////////////////////////////////////
  // clock_gettime(CLOCK_REALTIME) is not available on Mac OSX.
  ///////////////////////////////////////////////////////////////////////////////////////////
  void DAQtimer::DAQstart()
  {
    //clock_gettime(CLOCK_REALTIME,&tsStart);
    current_utc_time(&tsStart);
    tsctime1=tsStart;
    readcount =0;
  }
  void DAQtimer::readend()
  {
    //clock_gettime(CLOCK_REALTIME,&tsctime2);
    current_utc_time(&tsctime2);
    if(readcount<1000)
    {
      lltime_diff[readcount] = GetRealTimeInterval(&tsctime1,&tsctime2);
      tsctime1=tsctime2;
    }
    readcount++;
  }
  void DAQtimer::DAQend()
  {
    //clock_gettime(CLOCK_REALTIME,&tsEnd);
    current_utc_time(&tsEnd);
  }
  void DAQtimer::DAQsummary(int infreq)
  {
    unsigned long long llusec = GetRealTimeInterval(&tsStart,&tsEnd);
    std::cout <<llusec<<std::endl;
    double readfreq;
//    readfreq = (double)readcount/llusec*1000000.0;
//    std::cout<<readcount<<"events read. within "<<llusec/1000000.0<<"sec."<<std::endl;
    unsigned long long llRead = readcount * EVENTSIZE;
    readfreq = (double)readcount/llusec*1000000.0;
    readrate = (double)(llRead*8)/llusec*1000000.0/1024./1024.;
    std::cout<<readcount<<"events read. within "<<llusec/1000000.0<<"sec."<<std::endl;
    std::cout<<"Read Freq is "<<readfreq<<"[Hz]"<<std::endl;
    //**** file create
    m_foutName = MESFILE;
    m_fout.open(m_foutName,std::ios_base::in | std::ios_base::out | std::ios_base::ate);
    if (!m_fout) {
      m_fout.open(m_foutName);
      m_fout<<"The result of LSTDAQ \n";
      m_fout<<"InFreq[Hz]  RdFreq[Hz]  RdRate[Mbps] ";
      m_fout<<std::endl;
      std::cout<<"New measurement file is created. "<<std::endl;
    }
    m_fout//<<" "
    << std::setw(6)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)
    << infreq         << "     "
    << std::setw(10)  << std::setfill(' ')<< std::fixed<< std::setprecision(3)
    << readfreq       << "   "
    << std::setw(10)  << std::setfill(' ')<< std::fixed<< std::setprecision(3)
    << readrate       <<"\n";
    m_fout.close();
  }
}

