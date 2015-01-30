//****************************************************
// DAQ timer
//****************************************************
#include <time.h>     //for measuring time
#include <sys/time.h> //for making filename
#include "DAQtimer.hpp"
#include "Config.hpp" //EVENTSIZE
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
  void DAQtimer::DAQsummary(int infreq, 
			    unsigned long long NreadAll,
			    int nRB,
			    int nColl,
			    unsigned long Ntrg[MAX_CONNECTION],
			    unsigned long Nevt[MAX_CONNECTION])
  {
    //requisition time
    unsigned long long llreq_usec = GetRealTimeInterval(&tsStart,&tsEnd);
    //daq time (from 2nd to DAQ_NEVENTth events)
    unsigned long long lldaq_usec = llreq_usec - lltime_diff[0];
    std::cout<<"readcount:"<<readcount<<std::endl;
    std::cout<<"NreadAll :"<<NreadAll<<std::endl;
    std::cout<<"readcount*nRB="<<readcount*nRB<<std::endl;
    std::cout<<"duration for requisition :"<<llreq_usec<<"usec"<<std::endl;
    std::cout<<"duration for acquisition :"<<lldaq_usec<<"usec"<<std::endl;
    
    double readfreq = (double)(readcount-1)/(double)lldaq_usec*1000000.0;
    double readrate = (double)readfreq* (8.*(double)EVENTSIZE/1024./1024.);
    double thruput = readrate *(double)nRB/1024.;

    std::cout<<"Read Freq :"<<readfreq<<"[Hz]"<<std::endl;
    std::cout<<"Read Rate :"<<readrate<<"[Mbps]"<<std::endl;
    std::cout<<"Throughput:"<<thruput<<"[Gbps]"<<std::endl;
    
    
    //**** file create
    std::ofstream   m_fout;
    // using namespace std;
    // m_foutName = MESFILE;
    // char foutName[128];
    std::sprintf(m_foutName,MESFILE);
    m_fout.open(m_foutName,std::ios_base::in | std::ios_base::out | std::ios_base::ate);
    if (!m_fout) {
      m_fout.open(m_foutName);
      m_fout<<"The result of LSTDAQ \n";
      m_fout<<"nColl nConn InFreq[Hz]  RdFreq[Hz]  RdRate[Mbps]   NreadAll";
      for(int i=0;i<nRB;i++)
      {
        m_fout<<" Nevt["<< std::setw(2)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)<<i<<"]";
        m_fout<<" Ntrg["<< std::setw(2)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)<<i<<"]";
      }
      m_fout<<" readcount";
      m_fout<<std::endl;
      std::cout<<"New measurement file is created. "<<std::endl;
    }
    m_fout//<<" "
    << std::setw(4)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)
    << nColl            << "  "
    << std::setw(4)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)
    << nRB            << "  "
    << std::setw(6)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)
    << infreq         << "     "
    << std::setw(10)  << std::setfill(' ')<< std::fixed<< std::setprecision(3)
    << readfreq       << "   "
    << std::setw(10)  << std::setfill(' ')<< std::fixed<< std::setprecision(3)
    << readrate       << "   "
    << std::setw(8)  << std::setfill(' ')<< std::fixed<< std::setprecision(0)
    << NreadAll         << " ";
    for(int i=0;i<nRB;i++)
    {
      m_fout
      << std::setw(8)  << std::setfill(' ')<< std::fixed<< std::setprecision(0)
      << Nevt[i]         << " "
      << std::setw(8)  << std::setfill(' ')<< std::fixed<< std::setprecision(0)
      << Ntrg[i]         << " ";
    }
    m_fout
    << std::setw(8)  << std::setfill(' ')<< std::fixed<< std::setprecision(0)
    << readcount      <<"\n";
    m_fout.close();
  }
  
  void DAQtimer::DAQerrsummary(int infreq,
                               unsigned long long NreadAll,
			       int nRB,
                               unsigned long Ntrg[MAX_CONNECTION],
                               unsigned long Nevt[MAX_CONNECTION],
                               unsigned long NtrgSkip[MAX_CONNECTION],
                               unsigned long NevtSkip[MAX_CONNECTION])
  
  {
    
    
    //**** file create
    std::ofstream   m_fout;
    std::sprintf(m_foutName,ERRMESFILE);
    m_fout.open(m_foutName,std::ios_base::in | std::ios_base::out | std::ios_base::ate);
    if (!m_fout) {
      m_fout.open(m_foutName);
      m_fout<<"The result of LSTDAQ \n";
      m_fout<<"InFreq[Hz] readcount  NreadAll";
      for(int i=0;i<nRB;i++)
      {
        m_fout<<"   Nevt["<< std::setw(2)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)<<i<<"]";
        m_fout<<" NevtSkip["<< std::setw(2)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)<<i<<"]";
        m_fout<<"   Ntrg["<< std::setw(2)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)<<i<<"]";
        m_fout<<" NtrgSkip["<< std::setw(2)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)<<i<<"]";
      }
      m_fout<<std::endl;
      std::cout<<"New measurement file is created. "<<std::endl;
    }
    m_fout//<<" "
    << std::setw(6)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)
    << infreq         << " "
    << std::setw(11)  << std::setfill(' ')<< std::fixed<< std::setprecision(0)
    << readcount       << " "
    << std::setw(12)  << std::setfill(' ')<< std::fixed<< std::setprecision(0)
    << NreadAll         << " ";
    for(int i=0;i<nRB;i++)
    {
      m_fout
      << std::setw(11)  << std::setfill(' ')<< std::fixed<< std::setprecision(0)
      << Nevt[i]         << " "
      << std::setw(11)  << std::setfill(' ')<< std::fixed<< std::setprecision(0)
      << NevtSkip[i]         << " "
      << std::setw(11)  << std::setfill(' ')<< std::fixed<< std::setprecision(0)
      << Ntrg[i]         << " "
      << std::setw(11)  << std::setfill(' ')<< std::fixed<< std::setprecision(0)
      << NtrgSkip[i]         << " ";
    }
    m_fout
    << std::setw(8)  << std::setfill(' ')<< std::fixed<< std::setprecision(0)
    << readcount      <<std::endl;
    m_fout.close();

    
  }
  
  
  
  //MUST BE MODIFIED. NO LONGER UP TO DATE
  void DAQtimer::DAQerrend(int errRB,
			   int infreq, 
			   unsigned long long nEvent,
			   int nRB,
			   int nColl,
			   unsigned long Ntrg[MAX_CONNECTION],
			   unsigned long Nevt[MAX_CONNECTION])
  {
    //requisition time
    unsigned long long llreq_usec = GetRealTimeInterval(&tsStart,&tsEnd);
    //daq time (from 2nd to DAQ_NEVENTth events)
    unsigned long long lldaq_usec = llreq_usec - lltime_diff[0];
    
    // std::cout<<nEvent<<"events read."<<std::endl;
    // std::cout<<"duration for requisition :"<<llreq_usec<<"usec"<<std::endl;
    // std::cout<<"duration for acquisition :"<<lldaq_usec<<"usec"<<std::endl;
    
    double readfreq = (double)(nEvent-1)/(double)lldaq_usec*1000000.0;
    double readrate = (double)readfreq* (8.*(double)EVENTSIZE/1024./1024.);
    double thruput = readrate *(double)nRB/1024.;
    
    // std::cout<<"Read Freq :"<<readfreq<<"[Hz]"<<std::endl;
    // std::cout<<"Read Rate :"<<readrate<<"[Mbps]"<<std::endl;
    // std::cout<<"Throughput:"<<thruput<<"[Gbps]"<<std::endl;
    
    std::cout<<"ERROR: drop limit exceeded on RB"<<errRB<<std::endl;
    //**** file create
    std::ofstream   m_fout;
    // using namespace std;
    // m_foutName = MESFILE;
    // char foutName[128];
    std::sprintf(m_foutName,MESFILE);
    m_fout.open(m_foutName,std::ios_base::in | std::ios_base::out | std::ios_base::ate);
    if (!m_fout) {
      m_fout.open(m_foutName);
      m_fout<<"The result of LSTDAQ \n";
      m_fout<<"nColl nConn InFreq[Hz]  RdFreq[Hz]  RdRate[Mbps]   nEvents";
      for(int i=0;i<nRB;i++)
      {
        m_fout<<" Nevt["<< std::setw(2)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)<<i<<"]";
        m_fout<<" Ntrg["<< std::setw(2)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)<<i<<"]";
      }
      m_fout<<" ReadTrial";
      m_fout<<std::endl;
      std::cout<<"New measurement file is created. "<<std::endl;
    }
    m_fout<<"ERROR on RB"<<errRB
    << std::setw(4)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)
    << nColl            << "  "
    << std::setw(4)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)
    << nRB            << "  "
    << std::setw(6)<< std::setfill(' ')<< std::fixed<< std::setprecision(0)
    << infreq         << "     "
    << std::setw(10)  << std::setfill(' ')<< std::fixed<< std::setprecision(3)
    << readfreq       << "   "
    << std::setw(10)  << std::setfill(' ')<< std::fixed<< std::setprecision(3)
    << readrate       << "   "
    << std::setw(8)  << std::setfill(' ')<< std::fixed<< std::setprecision(0)
    << nEvent         << " ";
    for(int i=0;i<nRB;i++)
    {
      m_fout
      << std::setw(8)  << std::setfill(' ')<< std::fixed<< std::setprecision(0)
      << Nevt[i]         << " "
      << std::setw(8)  << std::setfill(' ')<< std::fixed<< std::setprecision(0)
      << Ntrg[i]         << " ";
    }
    m_fout
    << std::setw(8)  << std::setfill(' ')<< std::fixed<< std::setprecision(0)
    << readcount      <<"\n";
    m_fout.close();
  }


}

