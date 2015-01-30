#ifndef __DAQTIMER_H
#define __DAQTIMER_H
#define TIME_SEC2NSEC	1000000000


#include <fstream> //FILE discriptor

#include "Config.hpp"
//#include <time.h>     //for measuring time
//#include <sys/time.h> //for making filename

namespace LSTDAQ{
  class DAQtimer
  {
  public:
    DAQtimer(int nServ) throw();
    virtual ~DAQtimer() throw();
    
    unsigned long long GetRealTimeInterval(const  struct timespec *pFrom, const struct timespec *pTo);
    void DAQstart();
    void readend();
    void DAQend();
    void DAQsummary(int infreq, 
		    unsigned long long NreadAll,
		    int nRB,
		    int nColl,
		    unsigned long Ntrg[MAX_CONNECTION], 
		    unsigned long Nevt[MAX_CONNECTION]);
    void DAQerrsummary(int infreq,
                       unsigned long long NreadAll,
		       int nRB,
                       unsigned long Ntrg[MAX_CONNECTION],
                       unsigned long Nevt[MAX_CONNECTION],
                       unsigned long NtrgSkip[MAX_CONNECTION],
                       unsigned long NevtSkip[MAX_CONNECTION]);
                       
    void DAQerrend(int errRB,
		   int infreq,
		   unsigned long long nEvent,
		   int nRB,
		   int nColl,
		   unsigned long Ntrg[MAX_CONNECTION], 
		   unsigned long Nevt[MAX_CONNECTION]);
    void fclose();
  private:

    char   m_foutName[128];
    int m_nServ;
    struct timeval tv;
    struct timespec tsStart,tsEnd,tsRStart;
    struct timespec tsctime1,tsctime2;
    int readcount;
    unsigned long long llstartdiffusec;
    unsigned long long lltime_diff[1000];

  };
    
}
#endif
