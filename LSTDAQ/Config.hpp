//
//  Config.hpp
//  
//
//  Created by 石尾 一馬 on 2014/12/19.
//
//

#ifndef _Config_hpp
#define _Config_hpp

#define EVENTSIZE 976
#define HEADERLEN 2
#define IPADDRLEN 4
#define EVTNOLEN  4
#define TRGNOLEN  4

#define MAX_CONNECTION 48
#define MAX_RINGBUF 49

//error exit threshold 
#define ERR_NDROPPED 100000

//interval to measure throughput
#define THRUPUTMES_STARTSEC 1
#define THRUPUTMES_INTERVALSEC 1
#define THRUPUTMES_INTERVALNSEC 0 
//1000000 for 1msec
#define MAX_NWMES 10000

//outputfile by DAQtimer
#define MESFILE "LSTDAQmeasure.dat"
#define ERRMESFILE "LSTDAQerrmeasure.dat"
#endif
