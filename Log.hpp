#pragma once 
#include <iostream>
#include <string>

#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


using std::cout ;
using std::endl ;
using std::string;

#define notice  1
#define warning 2
#define error   3
#define fatal   4

enum ERR{
  Socketerr= 1,
  Binderr,
  Listenerr,
  Argerr
};

#define LOG(level, message) \
         Log(#level,message,__FILE__,__LINE__)

void Log(string level,string message,string filename,int line)
{
  struct timeval cur;
  gettimeofday(&cur,nullptr);
  int fd = open("log.txt",O_CREAT | O_WRONLY | O_APPEND);
  dup2(fd,1);    
  close(fd);
  cout<<"["<<level<<"] "<<"["<<message<<"] "<<"["<<cur.tv_sec <<"] "<<"["<<filename<<"] "<< "["<<line<<"] "<<endl;
}
