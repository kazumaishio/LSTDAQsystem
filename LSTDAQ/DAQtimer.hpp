#ifndef __DAQTIMER_H
#define __DAQTIMER_H
#define TIME_SEC2NSEC	1000000000
#define MESFILE "LSTDAQmeasure.dat"

#include <fstream> //FILE discriptor

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
    void DAQsummary(int infreq);
    void fclose();
  private:
    std::ofstream   m_fout;
    std::string   m_foutName;
    int m_nServ;
    struct timeval tv;
    struct timespec tsStart,tsEnd,tsRStart;
    struct timespec tsctime1,tsctime2;
    int readcount;
    int readrate;
    unsigned long long llstartdiffusec;
    unsigned long long lltime_diff[1000];

  };
    
}
#endif
