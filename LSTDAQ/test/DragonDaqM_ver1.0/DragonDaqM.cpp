///////////////////////////////////////////////////////////////////////////////////////////
// DragonDaqM.cpp
// Made from simpleread.cpp
// Modified by Kazuma Ishio Univ. of Tokyo
// Last update on 2014/07/28
//
// ****Function****
//  This program
//    (1)extracts datas from 2 Dragon clusters by TCP/IP connection.
//    (2)can save datas from them.
//    (3)measures throughput of taking data from cluster 1.
//    (4)can also measure each read() function for cluster 1.
//
// ****Usage****
// 1.Overwrite the following values:
//     (1)ip address and port number  (in Connection.conf)
//        These parameters are the ip address of the Dragon clusters.
//        Even if you rewrite ip address, you don't have to re-compile.
//     (2)lReadBytes          (in "Preparation of Data File")
//        This parameter defines how much you take data from cluster1.
//     (3)__g_buff[976]       (receive buffer)
//        This value defines how much you read as a event
//
// 2.If you want to measure each read() function,
//  uncomment the region from "Detailed measurement report output START"
//              to "Detailed measurement report output END".
//
// 3.Compile this with: 
//         ****************************************************
//         *       g++ -o DragonDaqM DragonDaqM.cpp -lrt        *
//         ****************************************************
// 4.Submit this with: 
//         **********************************************************
//         *    DragonDaq <ReadDepth> <InputFrequency> <DataCreate> *
//         **********************************************************
//     note1:Currently this doesn't work well unless you specify <ReadDepth> to be 30.
//     note2:To measure trigger rate, specify <InputFrequency> in the unit of [Hz].
//     note3:<DataCreate> is optional. 
//         **************************************************************
//         *If you want to create data files, set <DataCreate> to be 1. *
//         **************************************************************
///////////////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <iostream>
#include "termcolor.h"
#include <stdlib.h>

#include <time.h>     //for measuring time
#include <sys/time.h> //for making filename

#include <errno.h>
#include <ctime>
#include <unistd.h>   //for inspecting directory
#include <sys/stat.h> //for making directory

#include <fstream>
#include <sstream>

#include "DragonDaq.hh"

//unsigned char __g_buff[65535];//receive buffer
unsigned char __g_buff[976];//receive buffer
///////////////////////////////////////////////////////////////////////////////////////////
// main program
///////////////////////////////////////////////////////////////////////////////////////////

//eth_dragon sock[48];
 
int main(int argc, char *argv[])
{
  using namespace std;
  
  int rddepth;
  int infreq;
  int datacreate;

  /******************************************/
  //  Handling input arguments
  /******************************************/
  if(argc==3)
    {
      rddepth = atoi(argv[1]);
      infreq = atoi(argv[2]);
      datacreate = 0;
    }
  else if(argc==4)
    {
      rddepth = atoi(argv[1]);
      infreq = atoi(argv[2]);
      datacreate = atoi(argv[3]);
    }
  else
    {
      printf("usage: $./DragonDaq (ReadDepth) (InputFreq)\n");
      exit(EXIT_FAILURE);
    }
  TERM_COLOR_BLUE;
  printf("*********************************************\n");
  printf("*********************************************\n");
  printf("**                Dragon DAQ               **\n");
  printf("**          For multiple clusters          **\n");
  printf("**            K.Ishio 2014 July            **\n");
  printf("**                                         **\n");
  printf("**  Aquired Data:                          **\n");
  printf("**    clusterNN.dat from connection #NN    **\n");
  printf("**  Measurement Data:                      **\n");
  printf("**    Summary is  DragonDaq.dat            **\n");
  printf("**    Close inspection is in DragonDaqMes  **\n");
  printf("*********************************************\n");
  printf("*********************************************\n");
  TERM_COLOR_RESET;

 /******************************************/
  //  Difinitions for Close Inspection
  /******************************************/
  unsigned long long llstartdiffusec;
  unsigned long long lltime_diff[1000]={0};

  /******************************************/
  //  Reading Connection Configuration
  /******************************************/
  char szAddr[48][16];
  unsigned short shPort[48]={0};
  unsigned long lConnected[48] ={0};
  const char *ConfFile = "Connection.conf";
  std::ifstream ifs(ConfFile);
  std::string str;
  int nServ=0;
  while (std::getline(ifs,str)){
    if(str[0]== '#' || str.length()==0)continue;
    std::istringstream iss(str);
    iss >> szAddr[nServ] >> shPort[nServ];
    nServ++;
  }
  if(nServ>48){
    printf("The number of connections excessed limit.");
    exit(1);
  }
	
  // for(int i=0;i<48;i++)
  //   {
  //     sock[i] = ConnectTcp(eth_dragon[i].tcpAddr,eth_dragon[i].portnum, lConnected);
  //   }
  //	printf("come here %d\n",__LINE__);

  /******************************************/
  //  preparation of measurement summary file
  /******************************************/
  FILE *fp_ms;
  char outputfile[] ="DragonDaqM.dat";
  bool isnewfile;
  if ((fp_ms = fopen(outputfile,"r"))== NULL)
  {
      isnewfile=true;
  }
  else
  {
    std::string tempstr;
    int frddepth;
    fscanf(fp_ms, "The result of DragonDaqM RD%d\n",&frddepth);
    if(frddepth!=rddepth)
	{
	  printf("Confirm readdepth you specify and that in DragonDaq.dat\n");
	  exit(EXIT_FAILURE);
	}
  }

  if ((fp_ms = fopen(outputfile,"a"))== NULL)
  {
    printf("output file open error! exit");
    exit(EXIT_FAILURE);
  }
  else if(isnewfile)
  {
    fprintf(fp_ms,"The result of DragonDaqM RD%d\n",rddepth);
    fprintf(fp_ms,"InFreq[Hz] ");
    for(int i=0;i<nServ;i++)fprintf(fp_ms,"RdFreq%d[Hz] RdRate%d[Mbps] ",i,i);
    fprintf(fp_ms,"\n");
      
    //printf("result is NULL ,fp_ms = %d\n",fp_ms);
    //      std::cout<<"fp_ms="<<fp_ms<<std::endl;
  }
  else
  {
    printf("The file DragonDaq.dat already exists. Data will be added to it.\n");
    //printf("result is not NULL ,fp_ms = %d\n" , fp_ms);
  }

   /******************************************/
  // Preparation of Data File
  /******************************************/
  //Initialization of Data File
  char datafile[48][128];
  FILE *fp_d[48];
  for(int i =0;i<nServ;i++)
    {
      sprintf(datafile[i],"cluster%d.dat",i);      
      fp_d[i] = fopen(datafile[i],"wb");
    }
  //Difinition of Data Size
  unsigned long lReadBytes = 100000000; //data size to read.10^8(default val for DAQ tests)
  // unsigned long lReadBytes = 976*10000; //data size to read.~10^7
  // unsigned long lReadBytes = 10000; //data size to read.

  /******************************************/
  //  Connection Initialization
  /******************************************/
  int sock[48];
  int isconnect=0;
  for(int i =0;i<nServ;i++)
    {
      printf("read from server(%s:%u) %lu bytes\n",szAddr[i],shPort[i],lReadBytes);
      sock[i] = ConnectTcp(szAddr[i],shPort[i],lConnected[i]);
      printf("connection established\n");
      printf("lConnected[%d]=%lu\n",i,lConnected[i]);
      if(sock[i]<0)isconnect=1;
    }


  /******************************************/
  //  Data Extraction
  /******************************************/
  //int readcount = 0;
  //Maximum value of file discriptor
  int maxfd=sock[0];
  if(isconnect==0)
    {
      memset(__g_buff,0,sizeof(__g_buff));		
      fd_set fds, readfds;
      FD_ZERO(&readfds);
      for(int i=0;i<nServ;i++)FD_SET(sock[i], &readfds);
      for(int i=1;i<nServ;i++)
	{
	  if(sock[i]>maxfd)maxfd=sock[i];
	}

      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 10000;
      int n= 0;
      unsigned llRead[48] = {0};
      struct timespec tsStart,tsEnd,tsRStart;
      struct timespec tsctime1,tsctime2;
      int readcount =-1;
      clock_gettime(CLOCK_REALTIME,&tsStart);
      for(;;)
	{
	  memcpy(&fds,&readfds,sizeof(fd_set));
	  select(maxfd+1, &fds, NULL, NULL,&tv);
	  if(readcount==-1)
	    {
	      usleep(500000);
	      clock_gettime(CLOCK_REALTIME,&tsctime1);
	      tsRStart=tsctime1;
	      llstartdiffusec = GetRealTimeInterval(&tsStart,&tsctime1);
	      readcount++;
	    }
	  {
	    if( FD_ISSET(sock[0], &fds) )
	      {
		//printf("come here %d\n",__LINE__);
		int j=0;
		{
		  int n = read( sock[0],&__g_buff,sizeof(__g_buff));
		  //printf("cluster1 read %d \n ",n);
		  if(datacreate==1)
		    {
		      fwrite(__g_buff,n,1,fp_d[0]);
		    }
		  if(j<1000)
		    {
		      clock_gettime(CLOCK_REALTIME,&tsctime2);					     
		      lltime_diff[j] = GetRealTimeInterval(&tsctime1,&tsctime2);
		      tsctime1=tsctime2;
		      //cout<<j<<endl;
		    }
		  readcount++;
		  //if(readcount%100==0)printf("n=%d\n",n);
		  if( n> 0 )
		    {
		      llRead[0] += (unsigned long)n;
		      if( llRead[0] >= lReadBytes ) 
			{
			  clock_gettime(CLOCK_REALTIME,&tsEnd);
			  // if(datacreate==1)
			  //   {
			  //     n = read( sock1,&__g_buff,976-llRead[0]%976);
			  //     printf("%d\n",n);
			  //     fwrite(__g_buff,976-llRead[0]%976,1,fp_d1);
			  //   }
			  break;
			}
		    }else if( n == 0 ){}
		  else
		    {
		      fprintf(fp_ms,"read() from sock0 failed\n");
		      exit(1);
		  }
		  j++;
		}
	      }/**if(FD_ISSET(sock1,&fds))**/
	  }
	  // printf("come here %d\n",__LINE__);
	  for(int i=1;i<nServ;i++){
	    if( FD_ISSET(sock[i], &fds) )
	      {
		{
		  int n = read( sock[i],&__g_buff[0],sizeof(__g_buff));
		  //printf("cluster[%d] read %d Bytes\n ",i,n);
		  //		      readcount++;
		  //if(readcount%100==0)printf("n=%d\n",n);
		  if(datacreate==1)
		    {
		      fwrite(__g_buff,n,1,fp_d[i]);
		    }
		  if( n> 0 )
		    {
		      llRead[i] += (unsigned long)n;
		      // llRead[i] += (unsigned long)n;
		      // if( llRead[0] >= lReadBytes ) 
		      // 	{
		      // 	  clock_gettime(CLOCK_REALTIME,&tsEnd);
		      // 	  break;
		      // 	}
	            }else if( n == 0 ){}
		  else
		    {
		      fprintf(fp_ms,"read() from sock[%d] failed\n",i);
		      exit(1);
		    }
		}
	      }/**if(FD_ISSET(sock[i],&fds))**/
	  }/**for(i<nServ)**/
	}/**for(;;)**/
      //printf("readcount :%d\n",readcount);
      if(datacreate==1)
	{
	  printf("additionally fetched datas\n");
	  int n;
	  for(int i=0;i<nServ;i++)
	    {
	      n = read( sock[i],&__g_buff,976-llRead[i]%976);
	      printf("cluster[%d] : %d\n",i,n);
	      fwrite(__g_buff,n,1,fp_d[i]);
	    }
	}
      for(int i=0;i<nServ;i++)
	{
	  close(sock[i]);
	  fclose(fp_d[i]);
	}
      /******************************************/
      //  Measurement summary 
      /******************************************/
      unsigned long long llusec = GetRealTimeInterval(&tsRStart,&tsEnd);
      double * readfreq = new double[nServ];
      double * readrate = new double[nServ];
      for(int i=0;i<nServ;i++)
	{
	  readfreq[i] = (double)llRead[i]/(double)(16*(rddepth*2+1))/llusec*1000000.0;
	  readrate[i] = (double)(llRead[i]*8)/llusec*1000000.0/1024./1024.;
	}
      //output to measurement file
      fprintf(fp_ms,"%6d     ",infreq);
      for(int i=0;i<nServ;i++)
	  fprintf(fp_ms,"%10.3f   %10.3f   ",readfreq[i],readrate[i]);
      for(int i=0;i<nServ;i++)
	{
	  // fprintf(fp_ms,"%s ",szAddr[i]);

	  /*modified(1)*/
	  // char a[2];
	  // char b[16]="";
	  // strncpy(b,szAddr[i],16);
	  // strncpy(a,b+11,sizeof(b)-11);
	  // fprintf(fp_ms,"%s ",a);
	  
	  /*modified(2)*/
	  char a[2];
	  strncpy(a,szAddr[i]+11,sizeof(szAddr[i]-11));
	  fprintf(fp_ms,"%s ",a);

	}
      fprintf(fp_ms,"\n");
      //output to terminal
      for(int i=0;i<nServ;i++)
	printf("read bytes %s: %u /%llu = %gMbps\n",
	       szAddr[i]  ,llRead[i],llusec,(double)(llRead[i]*8)/llusec*1000000.0/1024./1024.);
      for(int i=0;i<nServ;i++)
	printf("read events %s: %6.1f , residual of %d bytes\n",
	       szAddr[i]  ,(double)llRead[i]/976.0,llRead[i]%976);
      printf("InFreq[Hz]  ReadFreq[Hz] DataSize[Bytes] ReadTime[us] ReadRate[Mbps] IPaddress\n");
      for(int i=0;i<nServ;i++){
      printf("%d      %g        %10d       %llu     %g    %s\n",
	     infreq, 
	     (double)llRead[i]/976.0/llusec*1000000.0,
	     llRead[i],
	     llusec,
	     (double)llRead[i]*8.0/llusec*1000000.0/1024.0/1024.0,
	     szAddr[i]
	     );
      }	      
      delete[] readfreq;
      delete[] readrate;
      /****************************************************/
      /***** Detailed measurement report output START *****/
      /****************************************************/
      // if(chdir("DragonDaqMes")==0)
      // 	{
      // 	  //printf("MeasurementData will be created in DragonDaqMes.\n");
      // 	}
      // else
      // 	{
      // 	  if(mkdir("DragonDaqMes",
      // 		   S_IRUSR|S_IWUSR|S_IXUSR|
      // 		   S_IRGRP|S_IWGRP|S_IXGRP|
      // 		   S_IROTH|S_IWOTH|S_IXOTH)==0)	
      // 	    {
      // 	      chdir("DragonDaqMes");
      // 	      //printf("Directory DragonDaqMes is created.\n");
      // 	      //printf("MeasurementData will be created in DragonDaqMes.\n");
      // 	    }
      // 	  else
      // 	    {
      // 	      printf("Directory creation error on creating Measurement file");
      // 	      return -1;
      // 	    }
      // 	}
      // time_t tnow;
      // struct tm *sttnow;
      // time(&tnow);
      // sttnow = localtime(&tnow);
      // char buf[126];
      // sprintf(buf,"infreq%d_%02d%02d_%02d%02d%02d.dat"
      // 	,infreq
      // 	,sttnow->tm_mon
      // 	,sttnow->tm_mday
      // 	,sttnow->tm_hour
      // 	,sttnow->tm_min
      // 	,sttnow->tm_sec);
      // FILE *fp_md;
      // fp_md = fopen(buf,"w");
      // //  fprintf(fp_md,"InFreq[Hz]  RdFreq[Hz] WrFreq[Hz] DataSize[Bytes] RdTime[us] WrTime[us] RdRate[Mbps] wst-rst[usec] wen-ren[usec]\n");
      // //  fprintf(fp_md,"%d      %g    %g       %d       %llu       %llu     %g       %llu       %llu\n",
      // fprintf(fp_md,"InFreq[Hz]  RdFreq[Hz] DataSize[Bytes] RdTime[us]  RdRate[Mbps] ctime1-Start[usec]\n");
      // fprintf(fp_md,"%d      %g       %d       %llu     %g       %llu\n",
      // 	infreq, 
      // 	(double)llRead/976.0/llusec*1000000.0,
      // 	llRead,
      // 	llusec,
      // 	(double)llRead*8.0/llusec*1000000.0/1024.0/1024.0,
      // 	llstartdiffusec
      // 	);
      // for(int i=0;i<1000;i++)
      //   {
      //     fprintf(fp_md,"%llu\n",lltime_diff[i]);
      //   }
      // fclose(fp_md);
      /****************************************************/
      /***** Detailed measurement report output END   *****/
      /****************************************************/
    }else{
    printf("can't connect to servert\n");
  }
  fclose(fp_ms);
}
///////////////////////////////////////////////////////////////////////////////////////////
// main program END
///////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////
// connect to a server(SiTCP)
///////////////////////////////////////////////////////////////////////////////////////////
int ConnectTcp(const char *pszHost, unsigned short shPort, unsigned long &lConnectedIP )
{
  int sockTcp = -1;
  struct sockaddr_in addrTcp;
  sockTcp = socket(AF_INET, SOCK_STREAM, 0);
  if( sockTcp < 0 ){
    perror("socket");
    return -1;
  }
  addrTcp.sin_family = AF_INET;
  addrTcp.sin_port = htons(shPort);
  addrTcp.sin_addr.s_addr = inet_addr(pszHost);
  if( addrTcp.sin_addr.s_addr == 0xffffffff ){
    struct hostent *host = gethostbyname(pszHost);
    if( host == NULL ){
      if( h_errno == HOST_NOT_FOUND ){
	printf("ConnectTcp() host not found : %s\n",pszHost);
      }else{
	printf("ConnectTcp() %s : %s\n",hstrerror(h_errno),pszHost);
	fprintf(stderr,"%s : %s\n",hstrerror(h_errno),pszHost);
      }
      return -2;
    }
    unsigned int **addrptr = (unsigned int **)host->h_addr_list;
    while( *addrptr != NULL){
      addrTcp.sin_addr.s_addr = *(*addrptr);
      if(connect(sockTcp,(struct sockaddr *)&addrTcp,sizeof(addrTcp)) ==0 )
	{			
	  lConnectedIP = (unsigned long )addrTcp.sin_addr.s_addr;
	  break;
	}
      addrptr++;
    }
    if( addrptr == NULL) {
      perror("ConnectTCP()::connect(1)");
      printf("ERROR:ConnectTCP:: host not found (%d) to %s:%u\n",sockTcp,pszHost,shPort);
      return -3;
    }else{
      printf("ConnectTCP::Connected0(%d) to %s:%u\n",sockTcp,pszHost,shPort);
    }
  }else{
    if(connect(sockTcp,(struct sockaddr *)&addrTcp,sizeof(addrTcp)) !=0 )
      {
	perror("ConnectTCP()::connect(2)");
	printf("ERROR:ConnectTCP:: can't connect not found (%d) to %08X %s:%u\n",addrTcp.sin_addr.s_addr,sockTcp,pszHost,shPort);
	sockTcp = -1;
	close(sockTcp);
	return -4;
      }else{
      lConnectedIP = (unsigned long )addrTcp.sin_addr.s_addr;
      printf("ConnectTCP::Connected1(%d) %s:%d\n",sockTcp,pszHost,shPort);
    }
  }
  return( sockTcp );
}


///////////////////////////////////////////////////////////////////////////////////////////
// time calc
///////////////////////////////////////////////////////////////////////////////////////////
#define TIME_SEC2NSEC	1000000000 
unsigned long long GetRealTimeInterval(const  struct timespec *pFrom, const struct timespec *pTo)
{
  unsigned long long  llStart = (unsigned long long )((unsigned long long )pFrom->tv_sec*TIME_SEC2NSEC + (unsigned long long )pFrom->tv_nsec);
  unsigned long long  llEnd = (unsigned long long )((unsigned long long )pTo->tv_sec*TIME_SEC2NSEC + (unsigned long long )pTo->tv_nsec);
  return( (llEnd - llStart)/1000 );
}


///////////////////////////////////////////////////////////////////////////////////////////
// ALL END
///////////////////////////////////////////////////////////////////////////////////////////


